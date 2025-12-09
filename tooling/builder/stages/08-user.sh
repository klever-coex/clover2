log_info "Install ZSH"
sudo apt-get install -yq \
    zsh \
    lolcat \
    figlet

sudo chsh -s $(which zsh)

log_info "Install Oh My ZSH"
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

echo "source /opt/ros/jazzy/setup.zsh" | tee -a /home/$USER/.zshrc
echo "source /home/$USER/clover2_ws/install/setup.zsh" | tee -a /home/$USER/.zshrc

log_info "Install ZSH settings"
cp $ASSETS_DIR/zprofile /home/$USER/.zprofile
