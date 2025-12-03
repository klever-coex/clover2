log_info "Build clover2 workspace"

cd /home/pi/clover2_ws
rosdep install --from-paths src --ignore-src -y
/bin/bash "source /opt/ros/$ROS_DISTRO/setup.bash && colcon build --symlink-install"

log_info "Install docker"
$REPO_DIR/tooling/scripts/generate_compose.py \
    --project-root $REPO_DIR \
    --version $VERSION \
    --output /opt/clover2
