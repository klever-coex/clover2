#! /usr/bin/env bash

set -ex

ROS_DISTRO=jazzy

apt update && apt install -y locales aptitude
locale-gen en_US en_US.UTF-8
update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

apt install -y software-properties-common
add-apt-repository -y universe

apt update && apt install -y curl

export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F\" '{print $4}')
curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb"
dpkg -i /tmp/ros2-apt-source.deb

apt update
apt install -y \
    ros-dev-tools \
    ros-${ROS_DISTRO}-ros-base \
    ros-${ROS_DISTRO}-mavros \
    ros-${ROS_DISTRO}-mavros-extras \
    ros-${ROS_DISTRO}-mavros-msgs \
    ros-${ROS_DISTRO}-web-video-server

wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | bash

source /opt/ros/${ROS_DISTRO}/setup.bash

EXTRA_ROS_WS=$(mktemp -d --suffix=.ros_ws)
cd $EXTRA_ROS_WS && mkdir src && cd src
git clone --branch 0.5.0 --detach https://github.com/christianrauch/camera_ros.git

cd .. && colcon build --install-base /opt/ros/${ROS_DISTRO}/ --merge-install
