log_info "Build clover2 workspace"

cd /home/$USER/clover2_ws
/bin/bash -c "cd /home/$USER/clover2_ws && source /opt/ros/$ROS_DISTRO/setup.bash && rosdep install -y --from-paths src --ignore-src"
/bin/bash -c "cd /home/$USER/clover2_ws && source /opt/ros/$ROS_DISTRO/setup.bash && colcon build --symlink-install"

log_info "Create /opt/clover2 dir"
sudo mkdir /opt/clover2
sudo chown -R $USER:$USER /opt/clover2

log_info "Setup docker compose"
# Pre-generate docker compose for pull
# DOCKER_COMPOSE_TMPDIR=$(mktemp -d)
# envsubst < $ASSETS_DIR/docker-compose.yaml.in > $DOCKER_COMPOSE_TMPDIR/docker-compose.yaml
# sudo docker compose --project-directory $DOCKER_COMPOSE_TMPDIR pull
# podman pull --arch=arm64 "${REGISTRY}clover2-gui:${CLOVER2_VERSION}"
# podman pull --arch=arm64 "${REGISTRY}clover2-docs:${CLOVER2_VERSION}"
# podman pull --arch=arm64 "${REGISTRY_HOST}/wettyoss/wetty"

log_info "Install udev rules"
sudo cp $ASSETS_DIR/udev/* /etc/udev/rules.d/

log_info "Install some scripts"
sudo cp $ASSETS_DIR/clover2_firstboot.sh /root/
sudo cp $ASSETS_DIR/ros2_launch.sh /opt/clover2/

sudo chmod +x /root/clover2_firstboot.sh
sudo chmod +x /opt/clover2/ros2_launch.sh

log_info "Install clover2 services"
sudo cp $ASSETS_DIR/systemd/* /etc/systemd/system/

sudo systemctl enable clover2.service
sudo systemctl enable clover2-web.service
sudo systemctl enable clover2-firstboot.service

log_info "Set image version ${CLOVER2_VERSION}"
echo "CLOVER2_VERSION=${CLOVER2_VERSION}" | sudo tee -a /usr/lib/os-release
