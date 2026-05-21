log_info "Remove snap"
sudo apt purge snapd -y

log_info "Upgrade system"
sudo apt-get update -qq >/dev/null
sudo apt-get upgrade -qy >/dev/null

log_info "Install common packages"
sudo apt-get install -qy \
    abi-compliance-checker \
    avahi-daemon \
    cmake \
    curl \
    libboost-dev \
    libjpeg-dev \
    liblttng-ust-dev \
    libnss-mdns \
    libpython3-dev \
    libssl-dev \
    libtiff-dev \
    libudev-dev \
    libunwind-dev \
    libyaml-dev \
    libyuv-dev \
    locales \
    lttng-tools \
    meson \
    network-manager \
    ninja-build \
    openssl \
    openssl \
    pkg-config \
    pybind11-dev \
    python3-jinja2 \
    lolcat \
    figlet \
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

cp /tmp/clover2-build-extras/deb/* /home/$USER/.clover2_backup/deb/

for fname in /tmp/clover2-build-extras/deb/*.deb; do
    log_info "Install deb: $fname"
    sudo apt install -f $fname
done

log_info "Install vscodium extension"
wget https://open-vsx.org/api/meta/pyrefly/linux-arm64/1.0.0/file/meta.pyrefly-1.0.0@linux-arm64.vsix -O /tmp/meta.pyrefly-1.0.0@linux-arm64.vsix
/opt/vscodium/bin/codium-server --install-extension /tmp/meta.pyrefly-1.0.0@linux-arm64.vsix

sudo apt-get autoclean
sudo apt-get clean
