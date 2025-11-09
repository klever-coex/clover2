
log_info "Update system"
DEBIAN_FRONTEND=noninteractive apt update >/dev/null
DEBIAN_FRONTEND=noninteractive apt install locales >/dev/null

log_info "Setup locales"
locale-gen en_US en_US.UTF-8 >/dev/null
update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 >/dev/null
export LANG=en_US.UTF-8

log_info "Adding ROS2 repository"
apt install -yq software-properties-common >/dev/null
add-apt-repository -y universe >/dev/null
export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F\" '{print $4}')
curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb"
dpkg -i /tmp/ros2-apt-source.deb
apt update >/dev/null
apt upgrade -yq >/dev/null

log_info "Install ROS2"
apt install -yq \
    ros-dev-tools \
    ros-$ROS_DISTRO-ros-core \
    ros-$ROS_DISTRO-ros-base \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool >/dev/null

log_info "Rosdep"
rosdep init || true
rosdep update
