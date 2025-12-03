import logging
import subprocess
import pathlib
import asyncssh
import asyncio
import random
from dataclasses import dataclass, field

from .component_base import ComponentBase

logger = logging.getLogger(__name__)

@dataclass
class QemuConfig:
    image: pathlib.Path
    ssh_user: str
    ssh_password: str
    ssh_port: int = field(default_factory=lambda: random.choice(range(2000, 4000)))
    machine: str = field(default="virt")
    smp: int = field(default=8)
    cpu: str = field(default="cortex-a57")
    ram_size: str = field(default="8G")
    extra_args: list[str] = field(default_factory=[])


class Qemu(ComponentBase):
    def __init__(self, cfg: QemuConfig):
        super().__init__()

        self.cfg = cfg
        self.qemu_args = ["qemu-system-aarch64"]

        self.configure_net()
        self.configure_drives()
        self.configure_system()

        self.qemu_args += self.cfg.extra_args

    async def copy_to(self, src, dest):
        async with asyncssh.connect('localhost', username=self.cfg.ssh_user, password=self.cfg.ssh_password, port=self.cfg.ssh_port, known_hosts=None) as conn:
            async with conn.start_sftp_client() as sftp:
                await sftp.put(src, dest, recurse=True)

    async def execute(self, cmd):
        async with asyncssh.connect('localhost', username=self.cfg.ssh_user, password=self.cfg.ssh_password, port=self.cfg.ssh_port, known_hosts=None) as conn:
            async with conn.create_process(cmd) as process:
                async for stdout_data in process.stdout:
                    logger.getChild("ssh").info(stdout_data.rstrip())

                async for stderr_data in process.stderr:
                    logger.getChild("ssh").error(stderr_data.rstrip())

                await process.wait()

    async def __aenter__(self):
        logger.info("Starting QEMU")

        self.qemu_task = asyncio.create_task(self.run_qemu())
        await self.wait_ssh()

        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        await self.execute("sudo shutdown now")
        await self.qemu_task

        logger.info("Exit from QEMU")

        return True

    def configure_net(self):
        logger.debug(f"Configure net...")

        self.qemu_args += ["-device", "virtio-net-device,netdev=net0"]
        self.qemu_args += ["-netdev", f"user,id=net0,hostfwd=tcp::{self.cfg.ssh_port}-:22,hostfwd=tcp::8080-:80"]

    def configure_drives(self):
        logger.debug(f"Configure drives...")
        self.qemu_args += ["-drive",
                           f"file={self.cfg.image},if=virtio,format=raw"]

    def configure_system(self):
        logger.debug(f"Configure system...")

        self.qemu_args += ["-machine", f"{self.cfg.machine}"]
        self.qemu_args += ["-cpu", f"{self.cfg.cpu}"]
        self.qemu_args += ["-smp", f"{self.cfg.smp}"]
        self.qemu_args += ["-m", f"{self.cfg.ram_size}"]
        self.qemu_args += ["-nographic"]

    def resize_image(self, image: pathlib.Path, new_size: str):
        logger.info(f"Resize image `{image}` to size {new_size}")
        subprocess.run(
            ["qemu-img", "resize", f"{image}", f"{new_size}"], check=True)

    async def run_qemu(self):
        logger.getChild("QEMU").debug(f"Run qemu {self.qemu_args}")

        self.qemu_process = await asyncio.create_subprocess_shell(
            " ".join(self.qemu_args),
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )

        async for stdout_data in self.qemu_process.stdout:
            logger.getChild("QEMU").info(stdout_data.decode().strip())

        async for stderr_data in self.qemu_process.stderr:
            logger.getChild("QEMU").error(stderr_data.decode().strip())

        logger.getChild("QEMU").info("finish")

    async def wait_ssh(self, timeout=120):
        retries = int(timeout / 5)
        while True:
            await asyncio.sleep(5)
            try:
                await self.execute("uname -a")
                break
            except Exception as e:
                logger.info(f"Wait ssh connection for 5s ({e})")

            if retries < 0:
                raise Exception(f"Failed to connect to ssh within {timeout}s")

            retries -= 1

        logger.info("SSH ready")
