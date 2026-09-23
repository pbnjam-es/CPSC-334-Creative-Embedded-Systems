#!/bin/bash
set -u

export GIT_TERMINAL_PROMPT=0
export GIT_SSH_COMMAND="ssh -o BatchMode=yes"

REPO_DIR="/home/stu3340/CPSC-334-Creative-Embedded-Systems"
DEST="$REPO_DIR/raspberrypi/config_files"
mkdir -p "$DEST"

# (2) record the IP
IP_ADDR=""
for i in $(seq 1 15); do
  IP_ADDR=$(hostname -I | awk '{print $1}')
  [ -n "$IP_ADDR" ] && break
  sleep 2
done
[ -z "$IP_ADDR" ] && IP_ADDR="unknown"

echo "The world's best Raspberry Pi has an address of: $IP_ADDR"
{
  echo "# Raspberry Pi IP"
  echo
  echo "Last updated: $(date '+%Y-%m-%d %H:%M:%S %Z')"
  echo
  echo "IP: $IP_ADDR"
} > "$DEST/ip.md"

# (1) copy config files 
for f in /boot/firmware/config.txt /boot/firmware/cmdline.txt; do
  if [ -f "$f" ]; then
    cp "$f" "$DEST/" && echo "Copied $f" || echo "FAILED to copy $f"
  else
    echo "Missing: $f"
  fi
done

# push to  gh every time
cd "$REPO_DIR" || { echo "Repo not found at $REPO_DIR"; exit 1; }
git add raspberrypi/config_files
if git diff --cached --quiet; then
  echo "No config changes to commit."
else
  git commit -m "configs update (auto!) $IP_ADDR" || { echo "Commit failed"; exit 1; }
  git push || { echo "Push failed"; sync; exit 1; }
  echo "Pushed."
fi

sync
