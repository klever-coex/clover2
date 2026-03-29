#!/usr/bin/env python3

import argparse
import asyncio
import logging
import pathlib
import re

import config
from components.chroot import Chroot, ChrootConfig
from components.qemu import Qemu, QemuConfig

logging.basicConfig(
    level=logging.DEBUG,
    format="[%(levelname)s][%(asctime)s]: %(name)s: %(message)s",
)

logger = logging.getLogger(__name__)


async def chroot_state(args, image: pathlib.Path):
    cfg = ChrootConfig(
        image,
        {
            1: "",  # mount to /
            0: "boot",  # mount to /boot
        },
        with_sudo=args.sudo,
    )

    async with Chroot(cfg) as chroot:
        await chroot.copy_to(
            config.PROJECT_DIR / "tooling/builder/assets/01-nopasswd", "etc/sudoers.d"
        )

        await chroot.copy_to(
            config.PROJECT_DIR / "tooling/builder/assets/user-data", "boot"
        )

        if config.DOCKER_OUTPUT_DIR.is_dir():
            for image in config.DOCKER_OUTPUT_DIR.glob("*.tar"):
                await chroot.copy_to(image, "root/")


async def qemu_state(args, image: pathlib.Path):
    cfg = QemuConfig(
        image=image,
        ssh_user="pi",
        ssh_password="raspberry",
        smp=16,
        extra_args=[
            "-append",
            '"console=ttyAMA0,115200 root=/dev/vda2 rw"',
            "-kernel",
            f"{config.PROJECT_DIR}/tooling/builder/assets/kernel-qemu-raspi4",
        ],
    )

    async with Qemu(cfg) as qemu:
        await qemu.execute("mkdir -p /home/pi/clover2_ws/src/clover2")

        for item in config.PROJECT_DIR.iterdir():
            if not re.match(r"^build.*$", item.name) and not item.name == ".venv":
                await qemu.copy_to(item, ("/home/pi/clover2_ws/src/clover2"))

        logger.info("Install make")
        await qemu.execute("sudo apt-get update && sudo apt-get install -y make")

        logger.info("Cleanup git project")
        await qemu.execute(
            "cd /home/pi/clover2_ws/src/clover2 && git reset --hard HEAD && git clean -fdx"
        )

        logger.info("Run image setup script")
        await qemu.execute(
            "cd /home/pi/clover2_ws/src/clover2 && make builder-image-setup"
        )


def parse_args():
    args = argparse.ArgumentParser()

    args.add_argument("--output", "-o", type=pathlib.Path, required=True)
    args.add_argument("--sudo", "-S", action="store_true")

    return args.parse_args()


async def main():
    args = parse_args()

    await chroot_state(args, args.output)
    await qemu_state(args, args.output)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt")
        exit(-1)
    except Exception as e:
        logger.error(f"Prepare fail: {e.with_traceback()}")
        exit(-1)
