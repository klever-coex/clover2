#! /usr/bin/env bash

echo "--- Fix home directory permissions"
chmod +rx /home/pi

/usr/bin/raspi-config nonint do_ssh 0

NEW_SSID='clover2-'$(head -c 100 /dev/urandom | xxd -ps -c 100 | sed -e "s/[^0-9]//g" | cut -c 1-6)

echo "--- Creating Wi-Fi AP with SSID=${NEW_SSID}"

NEW_HOSTNAME=$(echo ${NEW_SSID} | tr '[:upper:]' '[:lower:]')
echo "--- Setting hostname to $NEW_HOSTNAME"
hostnamectl set-hostname $NEW_HOSTNAME \
&& sed -i 's/127\.0\.1\.1.*/127.0.1.1\t'${NEW_HOSTNAME}' '${NEW_HOSTNAME}'.local/g' /etc/hosts

systemctl disable clover2-firstboot.service
rm /etc/systemd/system/clover2-firstboot.service
systemctl daemon-reload

rm /root/clover2_firstboot.sh

reboot
