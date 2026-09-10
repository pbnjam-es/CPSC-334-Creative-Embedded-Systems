#!/bin/bash
set -u

REPO_DIR="/home/stu3340/cpsc3340_ces"
DEST="$REPO_DIR/raspberrypi/config_files"
mkdir -p "$DEST"

# (2) record the IP
IP_ADDR=$(hostname -I | awk '{print $1}')
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
cd "$REPO_DIR" || exit 1
git add raspberrypi/config_files
if git diff --cached --quiet; then
  echo "No config changes to commit."
else
  git commit -m "configs update (auto!) $IP_ADDR"
  git push
fi
