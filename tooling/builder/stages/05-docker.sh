
log_info "Run get docker script"
curl -fsSL https://get.docker.com | sh

log_info "Fix docker permissions"
sudo usermod -aG docker $USER
