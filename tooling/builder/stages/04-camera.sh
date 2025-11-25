LIBCAMERA_VERSION="bfd68f786964636b09f8122e6c09c230367390e7"
CAMERA_ROS_VERSION="0.5.1"

camera_libcamera() {
    log_info "Install libcamera deps"
    sudo apt-get install -y \
        abi-compliance-checker \
        clang \
        cmake \
        libcrypto++-dev \
        libdw-dev \
        libevent-dev \
        libevent-dev \
        libexif-dev \
        libgstreamer-plugins-base1.0-dev \
        libgstreamer1.0-dev \
        libgtest-dev \
        libjpeg-dev \
        liblttng-ust-dev \
        libpython3-dev pybind11-dev \
        libssl-dev \
        libtiff-dev \
        libudev-dev \
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
    meson setup build --buildtype=release -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp -Dv4l2=enabled -Dgstreamer=enabled -Dtest=false -Dlc-compliance=disabled -Dcam=enabled -Dqcam=disabled -Ddocumentation=disabled -Dpycamera=enabled
    yes | sudo ninja -C build install
    sudo usermod -aG video $USER
}

camera_ros_support() {
    set +u
    log_info "Build ROS camera driver"

    if ! [ -v ROS_DISTRO ]; then
        log_error "ROS_DISTRO is not set"
    fi

    log_info "Clone ROS2 libcamera project"
    LIBCAMERA_ROS_WS=$(mktemp -d --suffix="-libcamera_ros")
    cd $LIBCAMERA_ROS_WS && mkdir src && cd src
    git clone --branch $CAMERA_ROS_VERSION --depth 1 https://github.com/christianrauch/camera_ros.git
    cd $LIBCAMERA_ROS_WS

    log_info "Install ROS2 libcamera deps"
    source /opt/ros/$ROS_DISTRO/setup.bash
    rosdep install -y --from-paths src --ignore-src --skip-keys=libcamera

    log_info "Build ROS2 libcamera packages"
    sudo su -c "source /opt/ros/$ROS_DISTRO/setup.bash && colcon build --install-base /opt/ros/$ROS_DISTRO/ --merge-install"
}

if ! command -v cam >/dev/null 2>&1; then
    log_info "Libcamera not installed"
    camera_libcamera
fi

camera_ros_support
