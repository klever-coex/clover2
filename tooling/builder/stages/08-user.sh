log_info "Edit sudoers"
echo "pi ALL=(ALL) NOPASSWD:ALL" | sudo tee /etc/sudoers.d/005_nopasswd

log_info "Install ZSH"
sudo apt install zsh -y
chsh -s $(which zsh)

log_info "Install Oh My ZSH"
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
