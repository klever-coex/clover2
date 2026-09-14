import logging
import pathlib
import subprocess

import docker

logger = logging.getLogger(__name__)


def fetch(image_names: tuple[str, ...], registry: str, tag: str,
          platform: str, out_dir: pathlib.Path) -> list[pathlib.Path]:
    client = docker.from_env()
    out_dir.mkdir(parents=True, exist_ok=True)

    tars = []
    for name in image_names:
        ref = f"{registry}{name}:{tag}"
        logger.info("Pulling %s (%s)", ref, platform)
        image = client.images.pull(ref, platform=platform)
        logger.info("Pulled %s", image.short_id)

        tar = out_dir / f"{name}.tar"
        logger.info("Saving -> %s", tar)
        subprocess.run(
            ["docker", "save", "--platform", platform, "-o", str(tar), ref],
            check=True)
        tars.append(tar)

    return tars
