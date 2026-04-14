LIBCAMERA_VERSION="9f4754cbc6f9ae2276c9e2f42db9302e64c723b5"

libcamera_install() {
    log_info "Install libcamera deps"
    sudo apt-get install -y \
        abi-compliance-checker \
        clang \
        cmake \
        libcrypto++-dev \
        libdw-dev \
        libevent-dev \
        libexif-dev \
        libgstreamer-plugins-base1.0-dev \
        libgstreamer1.0-dev \
        libgtest-dev \
        libjpeg-dev \
        liblttng-ust-dev \
        libpython3-dev \
        libssl-dev \
        libtiff-dev \
        libudev-dev \
        libunwind-dev \
        libyaml-dev \
        libyuv-dev \
        lttng-tools \
        meson \
        ninja-build \
        openssl \
        pkg-config \
        pybind11-dev \
        python3-jinja2 \
        python3-jinja2 \
        python3-ply \
        python3-yaml

    log_info "Clone libcamera project (ver: $LIBCAMERA_VERSION)"
    LIBCAMERA_SOURCE_DIR=$(mktemp -d --suffix="-libcamera")
    git clone https://github.com/raspberrypi/libcamera.git $LIBCAMERA_SOURCE_DIR
    cd $LIBCAMERA_SOURCE_DIR
    git checkout $LIBCAMERA_VERSION

    log_info "Build libcamera"
    meson setup build --prefix=/usr --buildtype=release -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp -Dv4l2=enabled -Dgstreamer=enabled -Dtest=false -Dlc-compliance=disabled -Dcam=enabled -Dqcam=disabled -Ddocumentation=disabled -Dpycamera=enabled
    sudo usermod -aG video $USER

    log_info "Create .deb package"
    DESTDIR=$(pwd)/libcamera ninja -C build install
    mkdir -p libcamera/DEBIAN
    cat <<EOF > libcamera/DEBIAN/control
Package: libcamera
Version: $(git describe --tags --exact-match | sed 's/^[^0-9]*//')
Section: libs
Priority: optional
Architecture: arm64
Maintainer: Lapin Matvey
Description: libcamera driver
EOF
    dpkg-deb --build libcamera

    log_info "Install .deb package"
    sudo apt install ./libcamera.deb -y
    cp ./libcamera.deb /home/$USER/.clover2_backup/deb
}

libcamera_install
