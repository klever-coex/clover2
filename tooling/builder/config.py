import os
import requests
import subprocess
import tempfile
import pathlib
from typing import Callable
from dataclasses import dataclass, field

assert os.environ.get("PROJECT_DIR") is not None, "PROJECT_DIR unset"

PROJECT_DIR: pathlib.Path = pathlib.Path(os.environ.get("PROJECT_DIR"))

UBUNTU_IMAGE_URL = "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz"

@dataclass
class ImageConfiguration:
    name: str
    base_image_url: str
    download_funcion: Callable
    build_dir: pathlib.Path = field(init=False)
    base_image_file: pathlib.Path = field(init=False)

    def __post_init__(self):
        self.build_dir = PROJECT_DIR / f"build-{self.name}-image"
        self.base_image_file = self.build_dir / (self.name + "-orig.img")

def clover2_ubuntu_24_04_download(cfg: ImageConfiguration):
    tmp_dir = pathlib.Path(tempfile.mkdtemp(prefix="clover2-downloads."))
    destination_path = tmp_dir / pathlib.Path(cfg.base_image_url).name

    with requests.get(cfg.base_image_url, stream=True) as r:
        r.raise_for_status()
        with open(destination_path, 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192):
                f.write(chunk)

    subprocess.run(["unxz", destination_path], check=True)
    image_path = destination_path.parent / destination_path.stem

    cfg.build_dir.mkdir(parents=True, exist_ok=True)



image_configurations: dict = {
    "ubuntu24-clover2": ImageConfiguration(
        "ubuntu24-clover2",
        "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz",
        clover2_ubuntu_24_04_download
    )
}
