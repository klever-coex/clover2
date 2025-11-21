import os
from pathlib import Path
from urllib.parse import urlparse
from enum import Enum
from dataclasses import dataclass, field

assert os.environ.get("PROJECT_DIR") is not None, "PROJECT_DIR unset"

PROJECT_DIR: Path = Path(os.environ.get("PROJECT_DIR"))


@dataclass
class ImageConfiguration:
    name: str
    base_image_url: str
    actions: list
    build_dir: Path = field(init=False)
    base_image_file: Path = field(init=False)

    def __post_init__(self):
        self.build_dir = PROJECT_DIR / f"build-{self.name}-image"
        self.base_image_file = self.build_dir / Path(self.base_image_url).stem


image_configurations: dict = {
    "clover2": ImageConfiguration(
        "clover2",
        "https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.3-preinstalled-server-arm64+raspi.img.xz",
        [
            # remove old cloud-init
            ("rm", Path("boot/user-data")),
            # copy cloud-init
            ("copy", Path("tooling/builder/assets/user-data"), Path("boot")),
            # copy project
            ("copy", PROJECT_DIR, Path("opt/")),
            # set nopasswd for pi
            ("copy", Path("tooling/builder/assets/01-nopasswd"), Path("/etc/sudoers.d")),
        ]
    )
}
