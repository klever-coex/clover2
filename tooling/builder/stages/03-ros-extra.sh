log_info "Install geographiclib"
sudo apt-get install -yq geographiclib-tools

log_info "Generate geographiclib"
wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

log_info "Install MAVROS"
sudo apt-get install -yq \
    ros-$ROS_DISTRO-mavros-msgs \
    ros-$ROS_DISTRO-mavros \
    ros-$ROS_DISTRO-mavros-extras \
    ros-$ROS_DISTRO-camera-info-manager \
    ros-$ROS_DISTRO-cv-bridge \
    ros-$ROS_DISTRO-image-view \
    ros-$ROS_DISTRO-image-geometry
