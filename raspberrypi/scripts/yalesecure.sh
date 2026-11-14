#!/bin/bash
# Adds YaleSecure to the Pi. Asks for your NetID and password, so neither is stored in this file or in git.
set -e
read -rp "NetID: " NETID
read -rsp "NetID password: " NETPW; echo

sudo nmcli connection delete YaleSecure >/dev/null 2>&1 || true
sudo nmcli connection add type wifi con-name YaleSecure ifname wlan0 ssid YaleSecure \
  wifi-sec.key-mgmt wpa-eap 802-1x.eap peap 802-1x.phase2-auth mschapv2 \
  802-1x.identity "$NETID" 802-1x.password "$NETPW"
sudo nmcli connection modify YaleSecure connection.autoconnect-priority 10
sudo nmcli connection up YaleSecure

echo "Connected. New IP:"
hostname -I