log_info "Cleanup image"

sudo apt-get autoclean -y
sudo apt-get clean -y

history -c && history -w
