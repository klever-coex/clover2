#! /usr/bin/env bash

source /root/.clover2-env

echo "---> Fix home directory permissions <---"
chmod +rx /home/pi

export CLOVER2_HOSTNAME='clover2-'$(openssl rand -hex 3)
REPO_DIR="/home/pi/clover2_ws/src/clover2"

echo "---> Creating Wi-Fi AP with SSID=${CLOVER2_HOSTNAME} <---"
nmcli con add type wifi ifname wlan0 mode ap con-name clover2 ssid $CLOVER2_HOSTNAME autoconnect true \
    && nmcli con modify clover2 802-11-wireless.band bg \
    && nmcli con modify clover2 ipv4.method shared ipv4.address 192.168.11.1/24 \
    && nmcli con modify clover2 ipv6.method disabled \
    && nmcli con modify clover2 wifi-sec.key-mgmt wpa-psk \
    && nmcli con modify clover2 wifi-sec.psk "cloverwifi"

echo "---> Setting hostname to $CLOVER2_HOSTNAME <---"
hostnamectl set-hostname $CLOVER2_HOSTNAME

echo "---> Generate /opt/clover2/docker-compose.yaml <---"
source /usr/lib/os-release
envsubst < $REPO_DIR/tooling/builder/assets/docker-compose.yaml.in > /opt/clover2/docker-compose.yaml
chown pi /opt/clover2/docker-compose.yaml

sudo systemctl enable codium-server@pi

echo "---> Remove firstboot scrip <---"
systemctl disable clover2-firstboot.service
rm /etc/systemd/system/clover2-firstboot.service
systemctl daemon-reload

echo "---> Install docker images <---"
for f in /root/*.tar; do
    cat $f | docker load
done

echo "---> Expand filesystem <---"
sudo raspi-config nonint do_expand_rootfs | true

rm /root/*.tar
rm /root/clover2_firstboot.sh

echo "---> Reboot <---"
reboot
