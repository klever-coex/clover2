log_info "Install netplan config"

sudo rm -rf /etc/netplan/50-cloud-init.yaml

sudo cp $ASSETS_DIR/10-rpi-netplan.yaml /etc/netplan/
sudo chmod 600 /etc/netplan/10-rpi-netplan.yaml

log_info "Apply netplan config"
sudo netplan generate
sudo netplan apply
