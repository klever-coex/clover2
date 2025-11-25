log_info "Install geographiclib"
sudo apt-get install -yq geographiclib-tools

wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

sudo apt-get install -yq \
    ros-$ROS_DISTRO-mavros-msgs \
    ros-$ROS_DISTRO-mavros \
    ros-$ROS_DISTRO-mavros-extras >/dev/null
