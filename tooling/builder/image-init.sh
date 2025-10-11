#! /usr/bin/env bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
PURPLE='\033[1;35m'
NC='\033[0m'

info() {
    echo -e "${GREEN}[  INFO  ]${NC} $1"
}

stage() {
    echo -e "${PURPLE}[  STAGE  ]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[  WARN  ]${NC} $1"
}

error() {
    echo -e "${RED}[  ERROR  ]${NC} $1"
    exit 1
}

stage "Create pi user"
echo 'pi:$6$c70VpvPsVNCG0YR5$l5vWWLsLko9Kj65gcQ8qvMkuOoRkEagI90qi3F/Y7rm8eNYZHW8CY6BOIKwMH7a3YYzZYL90zf304cAHLFaZE0' > /boot/userconf.txt

stage "Setting up SSH"
sed -i 's/^[#|]PasswordAuthentication [no|yes]*/PasswordAuthentication yes/g' /etc/ssh/sshd_config
ln -s /usr/lib/systemd/system/ssh.service \
      /usr/lib/systemd/system/sshd.service
ln -s /usr/lib/systemd/system/sshd.service \
      /etc/systemd/system/multi-user.target.wants/ssh.service

stage "Update distro"
apt-mark hold linux-image-raspi
apt-mark hold linux-raspi
apt update
apt upgrade -y

stage "Install extra tools"
apt install -y raspi-config meson cmake libboost-dev locales curl
