log_info "Build clover2 workspace"

CLOVER2_WS_DIR="/opt/clover2/ws"

cd "$CLOVER2_WS_DIR" || exit
/bin/bash -c "cd $CLOVER2_WS_DIR/src/clover2 && make clover2-devtool-install-repos"
/bin/bash -c "cd $CLOVER2_WS_DIR && source /opt/ros/$ROS_DISTRO/setup.bash && rosdep install -y --from-paths src --ignore-src --skip-keys=libcamera"
/bin/bash -c "cd $CLOVER2_WS_DIR && source /opt/ros/$ROS_DISTRO/setup.bash && colcon build --symlink-install"

log_info "Add clover2 project to bashrc"
echo "source $CLOVER2_WS_DIR/install/setup.bash" >> ~/.bashrc
cat >> ~/.bashrc <<'EOF'
clover2-settings() {
    ros2 run clover2_ui settings \
        "$(ros2 pkg prefix clover2_bringup --share)/schemas/klever5.yaml" \
        /opt/clover2/.config.yaml
}
EOF

log_info "Install udev rules"
sudo cp $ASSETS_DIR/udev/* /etc/udev/rules.d/

log_info "Install some scripts"
sudo cp $ASSETS_DIR/clover2_firstboot.sh /root/
sudo cp $ASSETS_DIR/ros2_launch.sh /opt/clover2/
cp $ASSETS_DIR/launcher_config.yaml /opt/clover2/.config.yaml
ln -s "$(ros2 pkg prefix clover2 --share)/examples" /home/$USER/examples

sudo chmod +x /root/clover2_firstboot.sh
sudo chmod +x /opt/clover2/ros2_launch.sh

log_info "Install clover2 services"
sudo cp $ASSETS_DIR/systemd/* /etc/systemd/system/

sudo systemctl enable clover2.service
sudo systemctl enable clover2-web.service
sudo systemctl enable clover2-firstboot.service

log_info "Set image version ${CLOVER2_VERSION}"
echo "CLOVER2_VERSION=${CLOVER2_VERSION}" | sudo tee -a /usr/lib/os-release
echo "CLOVER2_GIT_HASH=${CLOVER2_GIT_HASH}" | sudo tee -a /usr/lib/os-release

sudo chown -R $USER:$USER /opt/clover2
