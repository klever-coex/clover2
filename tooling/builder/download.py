#!/usr/bin/env python3

import argparse
import logging
import pathlib
import requests
import subprocess
import tempfile
import shutil

import config

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def download_image(args, cfg: config.ImageConfiguration):
    tmp_dir = pathlib.Path(tempfile.mkdtemp(prefix="clover2-downloads."))
    logger.info(f"Use tmp dir for downloads: '{tmp_dir}'")

    destination_path = tmp_dir / pathlib.Path(cfg.base_image_url).name

    logger.info(f"Downloading: '{cfg.base_image_url}'")
    with requests.get(cfg.base_image_url, stream=True) as r:
        r.raise_for_status()
        with open(destination_path, 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192):
                f.write(chunk)

    logger.info(f"Decompress...")
    subprocess.run(["unxz", "-T0", destination_path], check=True)

    image_path = destination_path.parent / destination_path.stem
    args.output.parent.mkdir(parents=True, exist_ok=True)

    logger.info(f"Copy image to '{args.output}'")
    if args.output.is_file():
        logger.info(f"Remove old `{args.output}`")
        args.output.unlink()

    shutil.copy(image_path, args.output)
    subprocess.run(["qemu-img", "resize", f"{args.output}", "8G"], check=True)


def parse_args():
    args = argparse.ArgumentParser()

    args.add_argument("--configuration", "-c",
                      choices=list(config.image_configurations.keys()), required=True)
    args.add_argument("--output", "-o", type=pathlib.Path, required=True)

    return args.parse_args()


def main(args):
    logger.info("Prepare '%s' image", args.configuration)
    cfg = config.image_configurations[args.configuration]

    download_image(args, cfg)


if __name__ == "__main__":
    try:
        main(parse_args())
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt")
        exit(-1)
    except Exception as e:
        logger.error(f"Prepare fail: {e}")
        exit(-1)
