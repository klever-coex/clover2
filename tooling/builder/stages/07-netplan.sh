log_info "Install netplan config"

sudo rm -rf /etc/netplan/50-cloud-init.yaml

sudo cp $ASSETS_DIR/10-clover2.yaml /etc/netplan/
sudo chmod 600 /etc/netplan/10-clover2.yaml

sudo netplan apply
