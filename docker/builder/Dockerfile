FROM ubuntu:noble AS kernel_builder

RUN apt update -y && \
    apt install -y \
        bc \
        bison \
        build-essential \
        flex \
        g++-aarch64-linux-gnu \
        gcc-aarch64-linux-gnu \
        git \
        libelf-dev \
        libssl-dev \
        make \
        ncurses-dev \
        wget \
        xz-utils

# RUN apt install qemu qemubuilder qemu-system-gui qemu-system-arm qemu-utils \
#         qemu-system-data qemu-system

WORKDIR /linux
RUN wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.34.tar.xz -O /tmp/linux.tar.xz && \
        tar xvf /tmp/linux.tar.xz -C /linux && \
        cd /linux/linux-6.1.34

WORKDIR /linux/linux-6.1.34

COPY ./docker/builder/qemu_kernel.config /tmp/qemu_kernel.config

RUN make defconfig
RUN ./scripts/kconfig/merge_config.sh  .config "/tmp/qemu_kernel.config"
RUN ARCH=arm64 CROSS_COMPILE=/bin/aarch64-linux-gnu- make

FROM ubuntu:noble

RUN apt update -y && \
    apt install -y \
        wget \
        xz-utils

WORKDIR /qemu
COPY --from=kernel_builder /linux/linux-6.1.34/arch/arm64/boot/Image /qemu/kernel-qemu
RUN wget --progress=dot:giga -O /qemu/image.img.xz \
    "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"
RUN unxz image.img.xz

RUN apt install -y qemu-system-arm qemu-utils \
    qemu-system-data qemu-system

CMD ["qemu-system-aarch64", \
    "-machine", "virt", \
    "-cpu", "cortex-a57", \
    "-smp", "4", \
    "-m", "8G", \
    "-kernel", "/qemu/kernel-qemu", \
    "-append", "console=ttyAMA0,115200 root=/dev/vda2 rw", \
    "-nographic", \
    "-drive", "file=/qemu/image.img,if=virtio,format=raw", \
    "-netdev", "user,id=net0,hostfwd=tcp::2222-:22,hostfwd=tcp::8080-:80", \
    "-device", "virtio-net-device,netdev=net0" \
]

# FROM debian:stable-slim

# RUN apt-get update && apt-get install -y qemu-system-arm && rm -rf /var/lib/apt/lists/*

# # Copy your downloaded RPI OS image, kernel, and DTB here
# COPY 2025-10-01-raspios-trixie-arm64-lite.img /opt/raspios.img
# COPY kernel-qemu-raspi4 /opt/kernel-qemu-raspi4
# COPY bcm2711-rpi-4-b.dtb /opt/bcm2711-rpi-4-b.dtb
# COPY qemu-system-aarch64 /opt/qemu-system-aarch64

# RUN apt-get update && apt-get install -y libjpeg8-dev && rm -rf /var/lib/apt/lists/*

# # Expose necessary ports if needed for SSH or other services
# # EXPOSE 22

# CMD ["/opt/qemu-system-aarch64", \
#         "-M", "raspi", \
#         "-kernel", "/opt/kernel-qemu-raspi4", \
#         "-dtb", "/opt/bcm2711-rpi-4-b.dtb", \
#         "-drive", "file=/opt/raspios.img,if=sd,format=raw", \
#         "-append", "root=/dev/mmcblk0p2 rw rootwait console=ttyAMA0,115200 console=ttyS0,115200", \
#         "-nographic", \
#         "-m", "4G"]
