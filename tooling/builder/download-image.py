#!/usr/bin/env python3

from pathlib import Path
import argparse
import logging
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
    decompress_base_image(args, cfg, downloaded_image)


if __name__ == "__main__":
    try:
        main(parse_args())
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt")
        exit(-1)
    except Exception as e:
        logger.error(f"Prepare fail: {e}")
        exit(-1)
