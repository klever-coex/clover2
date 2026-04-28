
log_info "Update system"
sudo apt-get update
sudo apt-get install -y locales

log_info "Setup locales"
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

log_info "Adding ROS2 repository"
sudo apt-get install -yq software-properties-common
sudo add-apt-repository -y universe
export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F\" '{print $4}')
curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb"
sudo dpkg -i /tmp/ros2-apt-source.deb

sudo apt-get update
sudo apt-get upgrade -yq

log_info "Install ROS2"
sudo apt-get install -yq \
    ros-dev-tools \
    ros-$ROS_DISTRO-ros-core \
    ros-$ROS_DISTRO-ros-base \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool

log_info "Rosdep"
sudo rosdep init
rosdep update

log_info "Add ROS2 source to bashrc"
echo "source /opt/ros/$ROS_DISTRO/setup.bash" >> ~/.bashrc

sudo apt-get autoclean
sudo apt-get clean
