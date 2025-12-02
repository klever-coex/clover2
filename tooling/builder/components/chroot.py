import logging
import subprocess
import pathlib
import tempfile
import asyncio
from typing import Dict
from dataclasses import dataclass, field

from .component_base import ComponentBase

logger = logging.getLogger(__name__)


@dataclass
class ChrootConfig:
    image: pathlib.Path
    fstab: Dict[int, pathlib.Path]
    mount_point: pathlib.Path = field(
        default_factory=lambda: pathlib.Path(tempfile.mkdtemp(prefix="clover2.")))
    with_sudo: bool = field(default=True)

    def __post_init__(self):
        self.image = pathlib.Path(self.image)
        self.mount_point = pathlib.Path(self.mount_point)

        for part in self.fstab.keys():
            self.fstab[part] = self.mount_point / \
                pathlib.Path(self.fstab[part])


class Chroot(ComponentBase):
    def __init__(self, cfg: ChrootConfig):
        super().__init__()

        self.cfg = cfg
        self.sudo = "sudo" if self.cfg.with_sudo else ""

    async def copy_to(self, src, dest):
        src = pathlib.Path(src)
        dest = pathlib.Path(self.cfg.mount_point / dest)

        if not src.is_file():
            raise Exception("Only files copy supported")

        logger.debug(f"Copying {src} to {dest}")
        loop = asyncio.get_running_loop()
        await loop.run_in_executor(
            None,
            lambda: subprocess.run([self.sudo, "cp", str(src), str(dest)], check=True),
        )

    async def execute(self, args):
        return await super().execute(args)

    async def __aenter__(self):
        loop = asyncio.get_running_loop()
        await loop.run_in_executor(None, self._open_image)
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        loop = asyncio.get_running_loop()
        await loop.run_in_executor(None, self._close_image)
        return False

    def _open_image(self):
        logger.debug(f'Opening image `{self.cfg.image}`')

        kpartx_stdout = subprocess.run(
            [self.sudo, "kpartx", "-asv", self.cfg.image], capture_output=True, text=True, check=True)

        loop_dev = []
        for info in kpartx_stdout.stdout.splitlines():
            loop_dev.append(pathlib.Path("/dev/mapper") / info.split()[2])

        for part, mnt in self.cfg.fstab.items():
            logger.debug(f'Mounting `{loop_dev[part]}` to `{mnt}`')
            subprocess.run(
                [self.sudo, "mount", loop_dev[part], mnt], check=True)

        logger.info("Successfully mounted.")

    def _close_image(self):
        logger.debug("Cleaning up and unmounting...")
        try:
            if self.cfg.mount_point.is_mount():
                logger.info(f"Unmounting {self.cfg.mount_point}...")
                subprocess.run(
                    [self.sudo, "umount", "--recursive", self.cfg.mount_point], check=True)

        except subprocess.CalledProcessError as e:
            logger.error(f"Error during unmounting: {e}")
        except FileNotFoundError:
            logger.error("Error: umount command not found.")

        try:
            logger.info("Deleting device maps...")
            subprocess.run([self.sudo, "kpartx", "-d",
                           self.cfg.image], check=True)
        except subprocess.CalledProcessError as e:
            logger.error(f"Error during kpartx deletion: {e}")
        except FileNotFoundError:
            logger.error("Error: kpartx command not found.")
