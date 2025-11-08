log_info "Install geographiclib"
sudo apt install -yq geographiclib-tools

wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

sudo apt install -yq \
    ros-$ROS_DISTRO-mavros-msgs \
    ros-$ROS_DISTRO-mavros \
    ros-$ROS_DISTRO-mavros-extras
