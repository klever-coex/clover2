#!/bin/bash
set -e # Exit on any error

# --- Configuration ---
TARGET_ROS_DISTRO="jazzy"
IMAGE_URL="https://cdimage.ubuntu.com/releases/24.04.3/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"
IMAGE_XZ="ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"
IMAGE_IMG="ubuntu-24.04.3-preinstalled-server-arm64+raspi.img"
WORK_DIR="$PWD/images"
MOUNT_DIR="$WORK_DIR/mount"

# --- Colors for output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# --- Check for root and dependencies ---
check_dependencies() {
    if [ "$EUID" -ne 0 ]; then
        error "Please run this script as root (sudo). Mounting requires it."
    fi

    command -v parted >/dev/null 2>&1 || error "parted is not installed. Install with 'sudo apt install parted'."
    command -v qemu-aarch64-static >/dev/null 2>&1 || error "qemu-user-static is not installed. Install with 'sudo apt install qemu-user-static'."
    command -v xz >/dev/null 2>&1 || error "xz-utils is not installed. Install with 'sudo apt install xz-utils'."
    info "All dependencies found."
}

# --- Setup workspace and download image ---
setup_workspace() {
    info "Creating workspace in $WORK_DIR"
    mkdir -p "$MOUNT_DIR"
    cd "$WORK_DIR"

    if [ ! -f "$IMAGE_XZ" ]; then
        info "Downloading Ubuntu 24.04 Server image for RPi 5..."
        wget "$IMAGE_URL" -O "$IMAGE_XZ" --show-progress
    else
        info "Image archive found, skipping download."
    fi

    if [ ! -f "$IMAGE_IMG" ]; then
        info "Extracting image..."
        xz -dk "$IMAGE_XZ"
    else
        info "Extracted image found, skipping extraction."
    fi
    info "Workspace setup complete."
}

# --- Mount the image partitions ---
mount_image() {
    info "Mounting image partitions..."

    # Find the sector offset of the root partition (usually partition 2)
    OFFSET=$(fdisk -l "$IMAGE_IMG" | grep "img2" | awk '{print $2 * 512}')
    if [ -z "$OFFSET" ]; then
        error "Could not find root partition offset."
    fi

    mount -o loop,offset=$OFFSET "$IMAGE_IMG" "$MOUNT_DIR" || error "Failed to mount root partition."

    # Bind mounts for chroot
    mount --bind /dev "$MOUNT_DIR/dev/"
    mount --bind /dev/pts "$MOUNT_DIR/dev/pts"
    mount --bind /proc "$MOUNT_DIR/proc"
    mount --bind /sys "$MOUNT_DIR/sys"
    mount --bind /run "$MOUNT_DIR/run"

    # Copy the QEMU static binary for ARM64 emulation
    cp /usr/bin/qemu-aarch64-static "$MOUNT_DIR/usr/bin/"
    info "Image mounted and prepared for chroot."
}

# --- Main chroot configuration function ---
configure_chroot() {
    info "Configuring image via chroot..."

    # Create the first-boot script inside the image
    cat > "$MOUNT_DIR/usr/local/bin/firstboot_ros2.sh" << 'EOF'
#!/bin/bash
set -e

# --- ROS 2 First-Boot Setup ---
echo "Starting ROS 2 first-boot configuration..."

# Source the ROS 2 environment (if installed via package manager)
source /opt/ros/jazzy/setup.bash

# Optional: Create a workspace directory and set permissions
mkdir -p /home/pi/ros2_ws/src
chown -R pi:pi /home/pi/ros2_ws

# Optional: Example - Clone a specific repository
# sudo -u pi bash -c 'cd /home/pi/ros2_ws/src && git clone https://github.com/ros2/example_nodes.git'

# --- Cleanup: Remove this service and script ---
systemctl disable firstboot-ros2.service
rm /etc/systemd/system/firstboot-ros2.service
rm /usr/local/bin/firstboot_ros2.sh
systemctl daemon-reload

echo "ROS 2 first-boot configuration complete. Service removed."
EOF

    chmod +x "$MOUNT_DIR/usr/local/bin/firstboot_ros2.sh"

    # Create the systemd service to run on first boot
    cat > "$MOUNT_DIR/etc/systemd/system/firstboot-ros2.service" << 'EOF'
[Unit]
Description=ROS 2 First Boot Configuration
After=network.target cloud-init.service
Wants=network.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/firstboot_ros2.sh
RemainAfterExit=no
User=root
Group=root
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

    # Now chroot and run the installation commands
    chroot "$MOUNT_DIR" /bin/bash <<- EOH
        set -e # Exit on error inside chroot

        # Setup locale and sources
        apt update
        apt install -y locales
        locale-gen en_US en_US.UTF-8
        update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
        export LANG=en_US.UTF-8

        # Add the ROS 2 repository
        apt install -y curl software-properties-common
        curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
        echo "deb [arch=\$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu \$(. /etc/os-release && echo \$UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null

        # Install ROS 2 packages (minimal desktop + development tools)
        apt update
        apt install -y --allow-downgrades  \
            ros-dev-tools \
            ros-jazzy-ros-base \
            python3-rosdep \
            python3-colcon-common-extensions \
            python3-argcomplete \
            build-essential

        # Initialize rosdep
        rosdep init || true # Ignore error if already initialized
        rosdep update

        # Enable the firstboot service
        systemctl enable firstboot-ros2.service

        # Set up the 'pi' user environment (optional)
        echo "source /opt/ros/jazzy/setup.bash" >> /home/pi/.bashrc
        echo "export ROS_DOMAIN_ID=0" >> /home/pi/.bashrc
        chown pi:pi /home/pi/.bashrc

        # Clean up apt cache to keep the image smaller
        apt autoremove -y
        apt clean
        rm -rf /var/lib/apt/lists/*
EOH

    info "Chroot configuration completed successfully."
}

# --- Unmount the image ---
unmount_image() {
    info "Unmounting image..."
    sync # Ensure all data is written to the image

    # Unmount bind mounts
    umount -l "$MOUNT_DIR/run"
    umount -l "$MOUNT_DIR/sys"
    umount -l "$MOUNT_DIR/proc"
    umount -l "$MOUNT_DIR/dev/pts"
    umount -l "$MOUNT_DIR/dev"

    # Unmount the root partition
    umount -l "$MOUNT_DIR" || error "Failed to unmount $MOUNT_DIR. Please check."

    # Remove QEMU binary from the image (cleanup)
    rm -f "$MOUNT_DIR/usr/bin/qemu-aarch64-static"

    info "Image unmounted successfully."
}

# --- Finalize and compress ---
finalize_image() {
    info "Finalizing new image..."
    cd "$WORK_DIR"

    NEW_IMAGE_NAME="ubuntu-24.04-server-ros2-${TARGET_ROS_DISTRO}-rpi5.img"
    mv "$IMAGE_IMG" "$NEW_IMAGE_NAME"

    info "Creating compressed image..."
    xz -z -T0 -v "$NEW_IMAGE_NAME" --keep

    info "${GREEN}Process complete!${NC}"
    info "Your new image is ready: ${GREEN}$WORK_DIR/${NEW_IMAGE_NAME}.xz${NC}"
    info "Flash it to an SD card using:"
    info "  sudo dd if=$WORK_DIR/${NEW_IMAGE_NAME}.xz | xz -d | dd of=/dev/sdX bs=4M status=progress"
    warn "Replace /dev/sdX with your SD card device (e.g., /dev/mmcblk0)!"
}

# --- Main script execution ---
main() {
    info "Starting Ubuntu 24.04 + ROS 2 Humble image creation for Raspberry Pi 5..."
    check_dependencies
    setup_workspace
    mount_image
    configure_chroot
    unmount_image
    finalize_image
}

# Run the main function
main "$@"
