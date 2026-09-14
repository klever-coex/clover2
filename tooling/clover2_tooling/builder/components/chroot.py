import asyncio
import logging
import pathlib
import subprocess
import tempfile
from dataclasses import dataclass, field
from typing import Dict

from .base import ComponentBase

logger = logging.getLogger(__name__)


@dataclass
class ChrootConfig:
    image: pathlib.Path
    fstab: Dict[int, pathlib.Path]
    mount_point: pathlib.Path = field(
        default_factory=lambda: pathlib.Path(tempfile.mkdtemp(prefix="clover2.")))
    with_sudo: bool = True

    def __post_init__(self):
        self.image = pathlib.Path(self.image)
        self.mount_point = pathlib.Path(self.mount_point)

        for part in self.fstab.keys():
            self.fstab[part] = self.mount_point / \
                pathlib.Path(self.fstab[part])


class Chroot(ComponentBase):
    def __init__(self, cfg: ChrootConfig):
        self.cfg = cfg
        self.sudo = "sudo" if self.cfg.with_sudo else ""

    async def copy_to(self, src, dest) -> None:
        src = pathlib.Path(src)
        dest = pathlib.Path(self.cfg.mount_point / dest)

        if not src.is_file():
            raise RuntimeError(f"Only files copy supported, got '{src}'")

        logger.debug("Copying %s to %s", src, dest)
        loop = asyncio.get_running_loop()
        await loop.run_in_executor(
            None,
            lambda: subprocess.run(
                [self.sudo, "cp", str(src), str(dest)], check=True),
        )

    async def execute(self, cmd: str, check: bool = True) -> None:
        raise RuntimeError("Chroot cannot execute commands inside the image")

    async def __aenter__(self) -> "Chroot":
        loop = asyncio.get_running_loop()
        await loop.run_in_executor(None, self._open_image)
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb) -> None:
        loop = asyncio.get_running_loop()
        await loop.run_in_executor(None, self._close_image)

    def _open_image(self):
        logger.debug("Opening image `%s`", self.cfg.image)

        kpartx_stdout = subprocess.run(
            [self.sudo, "kpartx", "-asv", self.cfg.image],
            capture_output=True, text=True, check=True)

        loop_dev = []
        for info in kpartx_stdout.stdout.splitlines():
            loop_dev.append(pathlib.Path("/dev/mapper") / info.split()[2])

        for part, mnt in self.cfg.fstab.items():
            logger.debug("Mounting `%s` to `%s`", loop_dev[part], mnt)
            subprocess.run(
                [self.sudo, "mount", loop_dev[part], mnt], check=True)

        logger.info("Successfully mounted.")

    def _close_image(self):
        logger.debug("Cleaning up and unmounting...")
        try:
            if self.cfg.mount_point.is_mount():
                logger.info("Unmounting %s...", self.cfg.mount_point)
                subprocess.run(
                    [self.sudo, "umount", "--recursive", self.cfg.mount_point], check=True)
        except subprocess.CalledProcessError as e:
            logger.error("Error during unmounting: %s", e)
        except FileNotFoundError:
            logger.error("umount command not found")

        try:
            logger.info("Deleting device maps...")
            subprocess.run([self.sudo, "kpartx", "-d",
                           self.cfg.image], check=True)
        except subprocess.CalledProcessError as e:
            logger.error("Error during kpartx deletion: %s", e)
        except FileNotFoundError:
            logger.error("kpartx command not found")
