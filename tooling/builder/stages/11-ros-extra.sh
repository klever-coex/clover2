log_info "Install geographiclib"
sudo apt-get install -yq geographiclib-tools

log_info "Generate geographiclib"
wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

log_info "Install MAVROS"
sudo apt-get install -yq \
    ros-$ROS_DISTRO-camera-info-manager \
    ros-$ROS_DISTRO-cv-bridge \
    ros-$ROS_DISTRO-diagnostics \
    ros-$ROS_DISTRO-image-geometry \
    ros-$ROS_DISTRO-image-view \
    ros-$ROS_DISTRO-mavros \
    ros-$ROS_DISTRO-mavros-extras \
    ros-$ROS_DISTRO-mavros-msgs \
    ros-$ROS_DISTRO-tf-transformations \
    ros-$ROS_DISTRO-web-video-server

log_info "Copy camera calibration files"
mkdir -p /home/$USER/.ros/camera_info
cp $ASSETS_DIR/camera_info/*.yaml /home/$USER/.ros/camera_info/

sudo apt-get autoclean
sudo apt-get clean
