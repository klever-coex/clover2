sudo apt-get update

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
    ros-$ROS_DISTRO-v4l2-camera \
    ros-$ROS_DISTRO-rmw-cyclonedds-cpp \
    ros-$ROS_DISTRO-web-video-server

log_info "Install clover2 depends"
/bin/bash -c "cd $CLOVER2_WS_DIR/src/clover2 && make clover2-devtool-install-repos"
/bin/bash -c "cd $CLOVER2_WS_DIR && source /opt/ros/$ROS_DISTRO/setup.bash && rosdep install -y --from-paths src --ignore-src --skip-keys=libcamera"
