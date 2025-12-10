log_info "Remove snap"
sudo apt purge snapd -y

log_info "Upgrade system"
sudo apt-get update -qq >/dev/null
sudo apt-get upgrade -qy >/dev/null

log_info "Install common packages"
sudo apt-get install -qy \
    avahi-daemon \
    cmake \
    curl \
    libboost-dev \
    libnss-mdns \
    locales \
    meson \
    network-manager \
    raspi-config
