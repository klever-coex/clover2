log_info "Remove snap"
sudo apt purge snapd -y

log_info "Upgrade system"
sudo apt-get update
sudo apt-get upgrade -qy

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
