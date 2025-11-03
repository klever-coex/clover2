LIBCAMERA_VERSION="0.5.1"

log_info "Install libcamera deps"
sudo apt install -y \
    clang \
    meson \
    ninja-build \
    pkg-config \
    libyaml-dev \
    python3-yaml \
    python3-ply \
    python3-jinja2 \
    openssl \
    libdw-dev \
    libunwind-dev \
    libudev-dev \
    libudev-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libpython3-dev pybind11-dev \
    libevent-dev \
    libtiff-dev \
    liblttng-ust-dev \
    python3-jinja2 \
    lttng-tools \
    libexif-dev \
    libjpeg-dev \
    pybind11-dev \
    libevent-dev \
    libgtest-dev \
    abi-compliance-checker

log_info "Clone libcamera project"
CAMERA_SOURCE_DIR=$(mktemp -d --suffix="-libcamera")
git clone --branch v$LIBCAMERA_VERSION --depth 1 https://github.com/raspberrypi/libcamera.git $CAMERA_SOURCE_DIR
cd $CAMERA_SOURCE_DIR

log_info "Build and install libcamera"
meson setup build --buildtype=release -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp -Dv4l2=enabled -Dgstreamer=enabled -Dtest=false -Dlc-compliance=disabled -Dcam=enabled -Dqcam=disabled -Ddocumentation=disabled -Dpycamera=enabled
ninja -C build install
sudo ninja -C build install
sudo usermod -aG video $USER

log_info "Build ROS camera support"
CAMERA_ROS_WS=$(mktemp -d --suffix=-camera_ros_ws)
cd $CAMERA_ROS_WS && mkdir src && cd src
git clone --branch $LIBCAMERA_VERSION --depth 1 https://github.com/christianrauch/camera_ros.git
cd $CAMERA_ROS_WS
colcon build --install-base /opt/ros/${ROS_DISTRO}/ --merge-install
