import os
import requests
import subprocess
import tempfile
import pathlib
from typing import Callable
from dataclasses import dataclass, field

assert os.environ.get("PROJECT_DIR") is not None, "PROJECT_DIR unset"

PROJECT_DIR: pathlib.Path = pathlib.Path(os.environ.get("PROJECT_DIR"))


@dataclass
class ImageConfiguration:
    name: str
    base_image_url: str
    build_dir: pathlib.Path = field(init=False)
    base_image_file: pathlib.Path = field(init=False)

    def __post_init__(self):
        self.build_dir = PROJECT_DIR / f"build-{self.name}-image"
        self.base_image_file = self.build_dir / (self.name + "-orig.img")


image_configurations: dict = {
    "clover2-ubuntu24": ImageConfiguration(
        "clover2-ubuntu24",
        "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"
    )
}
