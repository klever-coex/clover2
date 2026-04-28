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

log_info "Copy build variables into image"

CLOVER2_ENV_FILE="/root/.clover2-env"
echo "export REGISTRY=\"$REGISTRY\"" | sudo tee -a "$CLOVER2_ENV_FILE"
echo "export REGISTRY_HOST=\"$REGISTRY_HOST\"" | sudo tee -a "$CLOVER2_ENV_FILE"
echo "export CLOVER2_VERSION=\"$CLOVER2_VERSION\"" | sudo tee -a "$CLOVER2_ENV_FILE"
echo "export CLOVER2_GIT_HASH=\"$CLOVER2_GIT_HASH\"" | sudo tee -a "$CLOVER2_ENV_FILE"

# backup folder for builded packages and etc.
log_info "Create /home/$USER/.clover2_backup"
mkdir -p /home/$USER/.clover2_backup
mkdir -p /home/$USER/.clover2_backup/deb
