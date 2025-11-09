log_info "Install geographiclib"
apt install -yq geographiclib-tools

wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | bash

apt install -yq \
    ros-$ROS_DISTRO-mavros-msgs \
    ros-$ROS_DISTRO-mavros \
    ros-$ROS_DISTRO-mavros-extras >/dev/null
