log_info "Build clover2 workspace"

cd "$CLOVER2_WS_DIR" || exit
/bin/bash -c "cd $CLOVER2_WS_DIR && source /opt/ros/$ROS_DISTRO/setup.bash && colcon build --symlink-install --cmake-args -DBUILD_TESTING=0"

log_info "Add clover2 project to bashrc"
echo "source $CLOVER2_WS_DIR/install/setup.bash" >> ~/.bashrc

get_ros_pkg_share() {
    source "/opt/ros/${ROS_DISTRO}/setup.bash"
    source "${CLOVER2_WS_DIR}/install/setup.bash"
    ros2 pkg prefix "$1" --share
}

ln -s "$(get_ros_pkg_share clover2)/examples" /home/$USER/examples
cp -r "$(get_ros_pkg_share clover2_map)/map" /opt/clover2/map
