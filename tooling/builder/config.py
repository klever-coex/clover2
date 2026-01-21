import os
import requests
import subprocess
import tempfile
import pathlib
from typing import Callable
from dataclasses import dataclass, field

assert os.environ.get("PROJECT_DIR") is not None, "PROJECT_DIR unset"
assert os.environ.get("DOCKER_OUTPUT_DIR") is not None, "DOCKER_OUTPUT_DIR unset"

PROJECT_DIR: pathlib.Path = pathlib.Path(os.environ.get("PROJECT_DIR"))
DOCKER_OUTPUT_DIR: pathlib.Path = pathlib.Path(os.environ.get("DOCKER_OUTPUT_DIR"))


@dataclass
class ImageConfiguration:
    name: str
    base_image_url: str
    build_dir: pathlib.Path = field(init=False)

    def __post_init__(self):
        self.build_dir = PROJECT_DIR / f"build-{self.name}-image"


image_configurations: dict = {
    "clover2-ubuntu24": ImageConfiguration(
        "clover2-ubuntu24",
        "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"
    )
}
