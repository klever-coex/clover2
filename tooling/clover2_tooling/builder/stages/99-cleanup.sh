log_info "Cleanup image"

sudo apt-get autoclean
sudo apt-get clean

history -c && history -w
