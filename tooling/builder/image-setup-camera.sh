sudo apt install \
    clang \
    meson \
    ninja-build \
    pkg-config \
    libyaml-dev \
    python3-yaml \
    python3-ply \
    python3-jinja2 \
    openssl

sudo apt install \
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

git clone https://github.com/raspberrypi/libcamera.git
cd libcamera
meson setup build --buildtype=release -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp -Dv4l2=enabled -Dgstreamer=enabled -Dtest=false -Dlc-compliance=disabled -Dcam=enabled -Dqcam=disabled -Ddocumentation=disabled -Dpycamera=enabled
ninja -C build install
sudo ninja -C build install
sudo usermod -aG video $USER

cd

mkdir -p camera_ws/src
cd camera_ws/src
git clone --depth=1 --branch=0.5.1 https://github.com/christianrauch/camera_ros.git
cd ..
colcon build  --install-base /opt/ros/jazzy/ --merge-install
