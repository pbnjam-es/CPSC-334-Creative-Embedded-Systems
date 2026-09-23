#!/bin/bash
set -u

LAPTOP_USER="jielian"
LAPTOP_IP="10.66.74.64"
SRC="~/CPSC-334-Creative-Embedded-Systems/raspberrypi/config_files/ip.md"
DEST="~/Desktop/pi_backups/"

if scp -i ~/.ssh/id_ed25519 \
	-o BatchMode=yes -o ConnectTimeout=10 \
	"$SRC" "$LAPTOP_USER@$LAPTOP_IP:$DEST"; then
	echo "$(date '+%F %T') copied ip.md to $LAPTOP_IP"
else
	echo "$(date '+%F %T') copied ip.md to $LAPTOP_IP"
fi


