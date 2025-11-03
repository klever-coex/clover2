
log_info "Upgrade system"
sudo apt update -qq
sudo apt upgrade -qqy

log_info "Install common packages"
sudo apt install -qqy \
    raspi-config \
    meson \
    cmake \
    libboost-dev \
    locales \
    curl \
    avahi-daemon \
    libnss-mdns
