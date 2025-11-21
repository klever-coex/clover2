packer {
  required_plugins {
    qemu = {
      version = "~> 1"
      source  = "github.com/matveylapin/qemu"
    }
  }
}

source "qemu" "rpi5-ubuntu" {
  iso_url          = "file://${path.cwd}/build-clover2-image/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img"
  iso_checksum     = "none"
  # iso_url          = "file:///home/motya/.cache/packer/8ddd893ec7496b81105c55085d81bf9c77c992b8.iso"
  # iso_checksum     = "none"
  cdrom_interface  = "if=virtio,format=raw"
  disk_size        = "8G"
  use_backing_file = false
  efi_boot         = false
  format           = "raw"
  headless         = true
  net_device       = "virtio-net-device"
  qemu_binary      = "qemu-system-aarch64"
  ssh_password     = "raspberry"
  ssh_timeout      = "30m"
  ssh_username     = "pi"
  output_directory = "${path.cwd}/build-clover2-image-packer"

  qemuargs = [
    ["-machine", "virt"],
    ["-cpu", "cortex-a57"],
    ["-smp", "4"],
    ["-m", "8G"],
    ["-nographic"],
    ["-kernel", "${path.cwd}/tooling/builder/assets/kernel-qemu-raspi4"],
    ["-append", "console=ttyAMA0,115200 root=/dev/vda2 rw"]
  ]
}

build {
  name    = "clover2"
  sources = ["source.qemu.rpi5-ubuntu"]

  provisioner "shell" {
    inline = [
      "sudo apt update -y"
    ]
  }
}
