log_info "Remove snap"
sudo apt purge snapd -y

log_info "Upgrade system"
sudo apt-get update -qq >/dev/null
sudo apt-get upgrade -qy >/dev/null

log_info "Install common packages"
sudo apt-get install -qy \
    raspi-config \
    meson \
    cmake \
    libboost-dev \
    locales \
    curl \
    avahi-daemon \
    libnss-mdns
