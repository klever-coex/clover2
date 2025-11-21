#!/usr/bin/env python3

from pathlib import Path
import argparse
import logging
import lzma
import requests
import subprocess

import config

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def download_base_image(args, cfg: config.ImageConfiguration) -> Path:
    download_dir = cfg.build_dir

    logger.debug(f"Create dir '{download_dir}'")
    download_dir.mkdir(parents=True, exist_ok=True)

    destination_path = download_dir / Path(cfg.base_image_url).name

    if (destination_path.is_file() and args.skip):
        logger.info(f"Skipping downloading. Image already exist")
        return destination_path

    logger.debug(f"Download image to '{destination_path}'")
    with requests.get(cfg.base_image_url, stream=True) as r:
        r.raise_for_status()
        with open(destination_path, 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192):
                f.write(chunk)

    return destination_path


def decompress_base_image(args, cfg: config.ImageConfiguration, downloaded_image: Path) -> Path:

    if (cfg.base_image_file.is_file() and args.skip):
        logger.info(f"Skipping decompression")
        return cfg.base_image_file

    logger.debug(f"Decompressing file '{downloaded_image}'")
    subprocess.run(["unxz", "--keep", downloaded_image], check=True)

    return cfg.base_image_file


def process_image_actions(args, cfg: config.ImageConfiguration, root: Path):

    for action in cfg.actions:
        match action:
            case ("copy", from_dir, to_dir):
                logger.info(f"Coping {from_dir} to chroot:{to_dir}")
                if from_dir.is_file():
                    subprocess.run([
                        "sudo",
                        "cp",
                        f"{from_dir}",
                        f"{root / to_dir}/"
                    ], check=True)
                else:
                    subprocess.run([
                        "sudo",
                        "rsync",
                        "--update",
                        "-avz",
                        f"--exclude-from={config.PROJECT_DIR}/.gitignore",
                        f"{from_dir}",
                        f"{root / to_dir}/"
                    ], check=True)

            case ("rm", dir):
                logger.info(f"Remove {dir}")
                subprocess.run([
                    "sudo",
                    "rm",
                    "-rf",
                    f"{root / dir}",
                ], check=True)
            case _:
                logger.error(f"Invalid action {action}")


def prepare_image(args, cfg: config.ImageConfiguration, image: Path):
    mount_point = cfg.build_dir / "mnt"

    logger.debug(f"Create mount dir '{mount_point}'")
    mount_point.mkdir(parents=True, exist_ok=True)

    try:
        kpartx_stdout = subprocess.run(
            ["sudo", "kpartx", "-asv", image], capture_output=True, text=True, check=True)

        loop_dev = []
        for info in kpartx_stdout.stdout.splitlines():
            loop_dev.append(Path("/dev/mapper") / info.split()[2])

        # TODO: think about hardcode
        root_mount = mount_point
        logger.info(f"Mounting {loop_dev[1]} to {root_mount}...")
        subprocess.run(["sudo", "mount", loop_dev[1], root_mount], check=True)

        boot_mount = root_mount / "boot"
        logger.info(f"Mounting {loop_dev[0]} to {boot_mount}...")
        subprocess.run(["sudo", "mount", loop_dev[0], boot_mount], check=True)

        logger.info("Successfully mounted.")

        process_image_actions(args, cfg, mount_point)

    except subprocess.CalledProcessError as e:
        logger.error(f"An error occurred: {e}")
    except FileNotFoundError:
        logger.error("Error: kpartx or mount command not found")
    finally:
        logger.debug("Cleaning up and unmounting...")
        try:
            if mount_point.is_mount():
                logger.info(f"Unmounting {mount_point}...")
                subprocess.run(
                    ["sudo", "umount", "--recursive", mount_point], check=True)

        except subprocess.CalledProcessError as e:
            logger.error(f"Error during unmounting: {e}")
        except FileNotFoundError:
            logger.error("Error: umount command not found.")

        try:
            logger.info("Deleting device maps...")
            subprocess.run(["sudo", "kpartx", "-d", image], check=True)
        except subprocess.CalledProcessError as e:
            logger.error(f"Error during kpartx deletion: {e}")
        except FileNotFoundError:
            logger.error("Error: kpartx command not found.")


def parse_args():
    args = argparse.ArgumentParser()

    args.add_argument("--configuration", "-c",
                      choices=list(config.image_configurations.keys()), required=True)
    args.add_argument("--skip", "-s", action='store_true')

    return args.parse_args()


def main(args):
    logger.info("Prepare '%s' image", args.configuration)
    cfg = config.image_configurations[args.configuration]

    downloaded_image = download_base_image(args, cfg)
    image = decompress_base_image(args, cfg, downloaded_image)
    # prepare_image(args, cfg, image)


if __name__ == "__main__":
    try:
        main(parse_args())
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt")
        exit(-1)
    except Exception as e:
        logger.error(f"Prepare fail: {e}")
        exit(-1)
