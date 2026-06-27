#!/usr/bin/env python3

import argparse
import logging
import pathlib
import shutil
import subprocess
import tempfile

import config
import requests

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

CACHE_DIR = config.PROJECT_DIR / ".cache" / "downloads"


def _cached_file(url: str) -> pathlib.Path:
    filename = pathlib.Path(url).name
    return CACHE_DIR / filename


def _ensure_cached(cfg: config.ImageConfiguration) -> pathlib.Path:
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    cached = _cached_file(cfg.base_image_url)

    if cached.is_file():
        logger.info(f"Using cached: '{cached}'")
    else:
        logger.info(f"Downloading: '{cfg.base_image_url}' -> '{cached}'")
        with requests.get(cfg.base_image_url, stream=True) as r:
            r.raise_for_status()
            with open(cached, "wb") as f:
                for chunk in r.iter_content(chunk_size=8192):
                    f.write(chunk)

    return cached


def download_image(args, cfg: config.ImageConfiguration):
    tmp_dir = pathlib.Path(tempfile.mkdtemp(prefix="clover2-downloads."))
    logger.info(f"Use tmp dir for work: '{tmp_dir}'")

    cached = _ensure_cached(cfg)
    destination_path = tmp_dir / cached.name
    shutil.copy(cached, destination_path)

    logger.info("Decompress...")
    subprocess.run(["unxz", "-T0", destination_path], check=True)

    image_path = destination_path.parent / destination_path.stem
    args.output.parent.mkdir(parents=True, exist_ok=True)

    logger.info(f"Copy image to '{args.output}'")
    if args.output.is_file():
        logger.info(f"Remove old `{args.output}`")
        args.output.unlink()

    shutil.copy(image_path, args.output)
    subprocess.run(["qemu-img", "resize", f"{args.output}", "14G"], check=True)


def parse_args():
    args = argparse.ArgumentParser()

    args.add_argument(
        "--configuration",
        "-c",
        choices=list(config.image_configurations.keys()),
        required=True,
    )
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
