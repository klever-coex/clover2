#! /usr/bin/env bash

echo "---> Fix home directory permissions <---"
chmod +rx /home/pi

NEW_SSID='clover2-'$(openssl rand -hex 3)
NEW_HOSTNAME=$(echo ${NEW_SSID} | tr '[:upper:]' '[:lower:]')

echo "---> Creating Wi-Fi AP with SSID=${NEW_SSID} <---"
nmcli con add type wifi ifname wlan0 mode ap con-name clover2 ssid $NEW_SSID autoconnect true \
    && nmcli con modify clover2 802-11-wireless.band bg \
    && nmcli con modify clover2 ipv4.method shared ipv4.address 192.168.11.1/24 \
    && nmcli con modify clover2 ipv6.method disabled \
    && nmcli con modify clover2 wifi-sec.key-mgmt wpa-psk \
    && nmcli con modify clover2 wifi-sec.psk "cloverwifi"

echo "---> Setting hostname to $NEW_SSID <---"
hostnamectl set-hostname $NEW_SSID

echo "---> Remove firstboot scrip  <---"
systemctl disable clover2-firstboot.service
rm /etc/systemd/system/clover2-firstboot.service
systemctl daemon-reload

rm /root/clover2_firstboot.sh

echo "---> Reboot <---"
sudo reboot
