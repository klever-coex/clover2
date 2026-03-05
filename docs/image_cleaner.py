#!/usr/bin/env python3

import argparse
import logging
from pathlib import Path
from PIL import Image

FULL_HD = (1920, 1080)

VALID_EXTS = {".webp"}
SCAN_EXTS = {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".tiff", ".tif", ".svg"}

logger = logging.getLogger("img_cleaner")


def setup_logging(verbosity: int):
    level = logging.WARNING
    if verbosity == 1:
        level = logging.INFO
    elif verbosity >= 2:
        level = logging.DEBUG

    logging.basicConfig(
        level=level,
        format="%(levelname)s | %(message)s"
    )


def needs_processing(path: Path):
    ext = path.suffix.lower()

    if ext == ".svg":
        return True, "svg"

    try:
        with Image.open(path) as img:
            w, h = img.size
            logger.debug(f"{path} -> {w}x{h}")
    except Exception as e:
        logger.error(f"Failed reading {path}: {e}")
        return False, None

    if w > FULL_HD[0] or h > FULL_HD[1]:
        return True, f"bigger than fullhd ({w}x{h})"

    if ext not in VALID_EXTS:
        return True, f"not {', '.join(VALID_EXTS)} ({ext})"

    return False, None


def process_image(path: Path, dry_run: bool):
    flag, reason = needs_processing(path)
    if not flag:
        return False

    logger.info(f"Process: {path} | {reason}")

    if dry_run:
        return True

    if path.suffix.lower() == ".svg":
        logger.warning(f"Skipping SVG: {path}")
        return False

    try:
        with Image.open(path) as img:
            img = img.convert("RGBA")
            img.thumbnail(FULL_HD, Image.LANCZOS)

            out_path = path.with_suffix(".webp")
            img.save(out_path, "webp", quality=90, optimize=True)

        if out_path != path:
            path.unlink(missing_ok=True)

        logger.info(f"Converted -> {out_path}")

    except Exception as e:
        logger.error(f"Failed processing {path}: {e}")
        exit(-1)
    
    return True


def scan_directory(folder: Path, dry_run: bool):
    processed_images = 0
    for path in folder.rglob("*"):
        if path.is_file() and path.suffix.lower() in SCAN_EXTS:
            processed_images += process_image(path, dry_run)

    if (dry_run and processed_images):
        exit(-1)


def main():
    parser = argparse.ArgumentParser(
        description="Scan directory, downscale to FullHD and convert to JPG"
    )
    parser.add_argument("directory", type=Path)
    parser.add_argument("--dry-run", action="store_true", help="List changes only")
    parser.add_argument("-v", "--verbose", action="count", default=1)

    args = parser.parse_args()

    setup_logging(args.verbose)

    if args.dry_run:
        logger.info("Running in dry-run mode (no files modified)")

    scan_directory(args.directory, args.dry_run)


if __name__ == "__main__":
    main()
