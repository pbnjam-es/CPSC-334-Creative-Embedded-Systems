#!/bin/bash
# Runs at boot: records the Pi's IP + config files and pushes them to GitHub.
# Everything is logged to ~/ip_push.log so you can see what went wrong.
set -u

LOG=/home/stu3340/ip_push.log
exec >>"$LOG" 2>&1
echo "===== $(date '+%Y-%m-%d %H:%M:%S %Z') ====="

export HOME=/home/stu3340
export GIT_TERMINAL_PROMPT=0
export GIT_SSH_COMMAND="ssh -o BatchMode=yes -o StrictHostKeyChecking=accept-new"

REPO_DIR="/home/stu3340/CPSC-334-Creative-Embedded-Systems"
DEST="$REPO_DIR/raspberrypi/config_files"

cd "$REPO_DIR" || { echo "Repo not found at $REPO_DIR"; exit 1; }

# 1. Wait for a real IPv4 address (skip IPv6 and 169.254.x.x "no DHCP" addresses), up to 2 min
IP_ADDR=""
for i in $(seq 1 60); do
  IP_ADDR=$(hostname -I | tr ' ' '\n' | grep -E '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$' | grep -v '^169\.254\.' | head -n1)
  [ -n "$IP_ADDR" ] && break
  sleep 2
done
[ -z "$IP_ADDR" ] && IP_ADDR="unknown"
echo "IP: $IP_ADDR"

# 2. Wait until GitHub is actually reachable (having an IP != having internet), up to ~2.5 min
for i in $(seq 1 30); do
  if git ls-remote --exit-code origin HEAD >/dev/null 2>&1; then
    echo "GitHub reachable"
    break
  fi
  echo "GitHub not reachable yet (try $i)"
  sleep 5
done

# 3. Get up to date first, otherwise the push is rejected whenever you've pushed from your Mac
git pull --rebase --autostash || echo "Pull failed (continuing anyway)"

# 4. Record IP and copy configs
mkdir -p "$DEST"
{
  echo "# Raspberry Pi IP"
  echo
  echo "Last updated: $(date '+%Y-%m-%d %H:%M:%S %Z')"
  echo
  echo "IP: $IP_ADDR"
} > "$DEST/ip.md"

for f in /boot/firmware/config.txt /boot/firmware/cmdline.txt; do
  if [ -f "$f" ]; then
    cp "$f" "$DEST/" && echo "Copied $f" || echo "FAILED to copy $f"
  else
    echo "Missing: $f"
  fi
done

# 5. Commit and push (with retries)
git add raspberrypi/config_files
if git diff --cached --quiet; then
  echo "No changes to commit."
else
  git commit -m "configs update (auto!) $IP_ADDR" || { echo "Commit failed"; exit 1; }
fi

for i in 1 2 3; do
  if git push; then
    echo "Pushed."
    break
  fi
  echo "Push failed (try $i)"
  sleep 10
  git pull --rebase --autostash
done

sync