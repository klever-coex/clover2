# log_info "Edit sudoers"
# echo "pi ALL=(ALL) NOPASSWD:ALL" | sudo tee /etc/sudoers.d/005_nopasswd

# echo "PasswordAuthentication yes" | sudo tee /etc/ssh/sshd_config.d/10-allow-password.conf

log_info "Install ZSH"
sudo apt-get install zsh -y
chsh -s $(which zsh)

log_info "Install Oh My ZSH"
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
