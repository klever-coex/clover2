
log_info "Upgrade system"
DEBIAN_FRONTEND=noninteractive apt update -qq >/dev/null
DEBIAN_FRONTEND=noninteractive apt upgrade -qqy >/dev/null

log_info "Install common packages"
DEBIAN_FRONTEND=noninteractive apt install -qqy \
    raspi-config \
    meson \
    cmake \
    libboost-dev \
    locales \
    curl \
    avahi-daemon \
    libnss-mdns >/dev/null
