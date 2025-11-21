#!/usr/bin/env python3

import re
import pathlib
import argparse
import shutil
import tempfile
import logging
import asyncio
import subprocess

from components.chroot import ChrootConfig, Chroot
from components.qemu import QemuConfig, Qemu

import config

logging.basicConfig(
    level=logging.DEBUG,
    format='[%(levelname)s][%(asctime)s]: %(name)s: %(message)s',
)

logger = logging.getLogger(__name__)


def parse_args():
    args = argparse.ArgumentParser()

    args.add_argument("--image", "-i", type=pathlib.Path, required=True)
    args.add_argument("--output", "-o", type=pathlib.Path, required=True)
    args.add_argument("--sudo", "-S", action='store_true')

    return args.parse_args()


async def chroot_state(args, image: pathlib.Path, mount_point: pathlib.Path):
    cfg = ChrootConfig(
        image,
        mount_point,
        {
            1: "",
            0: "boot",
        },
        with_sudo=args.sudo
    )

    async with Chroot(cfg) as chroot:
        await chroot.copy_to(config.PROJECT_DIR /
                             "tooling/builder/assets/user-data", "boot")
        await chroot.copy_to(
            config.PROJECT_DIR / "tooling/builder/assets/01-nopasswd", "etc/sudoers.d")


async def qemu_state(args, image: pathlib.Path):
    cfg = QemuConfig(
        image,
        "pi",
        "raspberry",
        extra_args=[
            "-append", '"console=ttyAMA0,115200 root=/dev/vda2 rw"',
            "-kernel", f"{config.PROJECT_DIR}/tooling/builder/assets/kernel-qemu-raspi4"
        ]
    )

    async with Qemu(cfg) as qemu:
        await qemu.execute("mkdir -p /home/pi/clover2_ws/src/clover2")

        # for item in config.PROJECT_DIR.iterdir():
        #     if not re.match(r"^build.*$", item.name):
        #         await qemu.copy_to(item, ("/home/pi/clover2_ws/src/clover2"))

        # await qemu.execute("/home/pi/clover2_ws/src/clover2/tooling/builder/image-setup.sh")

async def main():
    args = parse_args()

    logger.info(f"Copy image to '{args.output}'")

    # TODO: move to download component
    shutil.copy(args.image, args.output)
    subprocess.run(["qemu-img", "resize", f"{args.output}", "8G"], check=True)

    await chroot_state(args, args.output, pathlib.Path(
        tempfile.mkdtemp(prefix="clover2.")))

    # await qemu_state(args, args.output)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt")
        exit(-1)
    except Exception as e:
        logger.error(f"Prepare fail: {e.with_traceback()}")
        exit(-1)
