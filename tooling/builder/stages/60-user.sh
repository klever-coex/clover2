log_info "Install Shell Tools"
sudo apt-get install -yq \
    lolcat \
    figlet

log_info "Install motd scripts"
sudo rm -rf /etc/update-motd.d/*
sudo cp $ASSETS_DIR/motd/* /etc/update-motd.d/
sudo chmod +x /etc/update-motd.d/*
