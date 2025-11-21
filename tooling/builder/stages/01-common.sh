
log_info "Upgrade system"
sudo apt update -qq >/dev/null
sudo apt upgrade -qqy >/dev/null

log_info "Install common packages"
sudo apt install -qqy \
    raspi-config \
    meson \
    cmake \
    libboost-dev \
    locales \
    curl \
    avahi-daemon \
    libnss-mdns >/dev/null
