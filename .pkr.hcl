source "qemu" "rpi5-ubuntu" {
  iso_url          = "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"
  iso_checksum     = "sha256:..."
  output_directory = "output"
  disk_size        = "8G"
  format           = "raw"
  headless         = true
  
  # Настройки для aarch64
  qemuargs = [
    ["-machine", "virt"],
    ["-cpu", "cortex-a72"],
    ["-smp", "4"],
    ["-m", "4096"],
    ["-device", "virtio-gpu-pci"],
    ["-device", "qemu-xhci"],
    ["-device", "usb-kbd"],
    ["-device", "usb-mouse"],
  ]
  
  ssh_username = "ubuntu"
  ssh_password = "ubuntu"
  ssh_timeout  = "30m"
}

build {
  sources = ["source.qemu.rpi5-ubuntu"]
  
  # Копирование исходников
  provisioner "file" {
    source      = "../my-project"
    destination = "/home/ubuntu/"
  }
  
  # Запуск скрипта установки
  provisioner "shell" {
    script = "scripts/setup.sh"
  }
  
  # Или inline команды
  provisioner "shell" {
    inline = [
      "cd /home/ubuntu/my-project",
      "chmod +x install.sh",
      "sudo ./install.sh"
    ]
  }
}
