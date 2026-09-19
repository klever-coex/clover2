import logging
import pathlib
import shutil
import subprocess
import tempfile

import requests

from clover2_tooling.builder.config import ImageConfiguration

logger = logging.getLogger(__name__)

CHUNK_SIZE = 1024 * 1024
PROGRESS_STEP = 100 * 1024 * 1024  # log every 100 MiB


def _ensure_cached(url: str, cache_dir: pathlib.Path) -> pathlib.Path:
    cache_dir.mkdir(parents=True, exist_ok=True)
    cached = cache_dir / pathlib.Path(url).name

    if cached.is_file():
        logger.info("Using cached: '%s'", cached)
        return cached

    logger.info("Downloading: '%s' -> '%s'", url, cached)
    with requests.get(url, stream=True, timeout=60) as r:
        r.raise_for_status()
        total = 0
        with open(cached, "wb") as f:
            for chunk in r.iter_content(chunk_size=CHUNK_SIZE):
                f.write(chunk)
                total += len(chunk)
                if total // PROGRESS_STEP != (total - len(chunk)) // PROGRESS_STEP:
                    logger.info("Downloaded %d MiB...", total // (1024 * 1024))
    return cached


def download(cfg: ImageConfiguration, output: pathlib.Path,
             cache_dir: pathlib.Path) -> pathlib.Path:
    tmp_dir = pathlib.Path(tempfile.mkdtemp(prefix="clover2-downloads."))
    logger.info("Use tmp dir for work: '%s'", tmp_dir)

    cached = _ensure_cached(cfg.base_image_url, cache_dir)
    compressed = tmp_dir / cached.name
    shutil.copy(cached, compressed)

    logger.info("Decompressing...")
    subprocess.run(["unxz", "-T0", compressed], check=True)

    image_path = compressed.parent / compressed.stem
    output.parent.mkdir(parents=True, exist_ok=True)

    if output.is_file():
        logger.info("Remove old '%s'", output)
        output.unlink()

    logger.info("Copy image to '%s'", output)
    shutil.copy(image_path, output)
    subprocess.run(["qemu-img", "resize", str(output), cfg.size], check=True)
    return output
