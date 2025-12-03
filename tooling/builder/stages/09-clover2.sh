log_info "Build clover2 workspace"

cd /home/pi/clover2_ws
rosdep install --from-paths src --ignore-src -y
/bin/bash "source /opt/ros/$ROS_DISTRO/setup.bash && colcon build --symlink-install"

log_info "Install docker"
sudo python3 $REPO_DIR/tooling/scripts/generate_compose.py \
    --project-root $REPO_DIR \
    --version "$CLOVER2_VERSION" \
    --clover2-docs \
    --clover2-gui \
    --clover2-wetty \
    --output /opt/clover2/docker-compose.yaml

# sudo docker compose --project-directory /opt/clover2 pull
