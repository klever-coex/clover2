log_info "Install ZSH"
sudo apt-get install -yq \
    zsh \
    lolcat \
    figlet

sudo chsh -s $(which zsh)

log_info "Install Oh My ZSH"
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

log_info "Install ZSH settings"
cp $ASSETS_DIR/zshrc /home/$USER/.zshrc

log_info "Install motd scripts"
sudo rm -rf /etc/update-motd.d/*
sudo cp $ASSETS_DIR/motd/* /etc/update-motd.d/
