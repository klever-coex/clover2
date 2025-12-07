log_info "Install ZSH"
sudo apt-get install -yq \
    zsh \
    lolcat \
    figlet

sudo chsh -s $(which zsh)

log_info "Install ZSH settings"
cp $ASSETS_DIR/zprofile /home/$USER/.zprofile
