
log_info "Download docker installation script"
curl -fsSL https://get.docker.com -o /tmp/docker-install.sh

log_info "Run docker install script"
chmod +x /tmp/docker-install.sh
/bin/bash /tmp/docker-install.sh

log_info "Fix docker permissions"
sudo usermod -aG docker $USER

log_info "Login to registry"
/bin/bash "echo $DOCKER_REGISTRY_PASSWORD | sudo docker login $REGISTRY_HOST --username $DOCKER_REGISTRY_USER --password-stdin"
