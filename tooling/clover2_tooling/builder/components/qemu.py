import asyncio
import logging
import pathlib
import random
from dataclasses import dataclass, field

import asyncssh

from .base import ComponentBase

logger = logging.getLogger(__name__)

SSH_POLL_INTERVAL = 5


@dataclass
class QemuConfig:
    image: pathlib.Path
    ssh_user: str
    ssh_password: str
    ssh_port: int = field(default_factory=lambda: random.randint(2000, 3999))
    machine: str = "virt"
    smp: int = 8
    cpu: str = "cortex-a57"
    ram_size: str = "16G"
    extra_args: tuple[str, ...] = ()


class Qemu(ComponentBase):
    def __init__(self, cfg: QemuConfig):
        self.cfg = cfg
        self._conn: asyncssh.SSHClientConnection | None = None
        self.qemu_task: asyncio.Task | None = None
        self.qemu_process: asyncio.subprocess.Process | None = None
        self.is_running = False

        self.qemu_args: list[str] = ["qemu-system-aarch64"]
        self._configure_net()
        self._configure_drives()
        self._configure_system()
        self.qemu_args += list(self.cfg.extra_args)

    async def copy_to(self, src, dest) -> None:
        async with self._conn.start_sftp_client() as sftp:
            await sftp.put(src, dest, recurse=True)

    async def execute(self, cmd: str, check: bool = True) -> None:
        async with self._conn.create_process(cmd) as process:
            async for line in process.stdout:
                logger.getChild("ssh").info(line.rstrip())
            async for line in process.stderr:
                logger.getChild("ssh").error(line.rstrip())
            await process.wait()

            if check and process.exit_status != 0:
                raise RuntimeError(
                    f"Command failed ({process.exit_status}): {cmd}")

    async def __aenter__(self) -> "Qemu":
        logger.info("Starting QEMU (ssh port %s)", self.cfg.ssh_port)
        self.is_running = True
        self.qemu_task = asyncio.create_task(self._run_qemu())
        await self._wait_ssh()
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb) -> None:
        self.is_running = False
        try:
            if exc_type is None:
                await asyncio.wait_for(
                    self.execute("sudo shutdown now", check=False), timeout=30)
        except (Exception, TimeoutError) as e:
            logger.warning("Graceful shutdown failed: %s", e)
        finally:
            if self._conn:
                self._conn.close()
                try:
                    await asyncio.wait_for(self._conn.wait_closed(), timeout=10)
                except (Exception, TimeoutError):
                    logger.warning("SSH connection did not close cleanly")

            if self.qemu_process and self.qemu_process.returncode is None:
                self.qemu_process.terminate()
                try:
                    await asyncio.wait_for(self.qemu_process.wait(), timeout=10)
                except TimeoutError:
                    logger.warning("QEMU did not stop, killing")
                    self.qemu_process.kill()
                    await self.qemu_process.wait()

            if self.qemu_task:
                await self.qemu_task

            logger.info("QEMU finished")

    def _configure_net(self) -> None:
        self.qemu_args += [
            "-device", "virtio-net-device,netdev=net0",
            "-netdev", f"user,id=net0,hostfwd=tcp::{self.cfg.ssh_port}-:22",
        ]

    def _configure_drives(self) -> None:
        self.qemu_args += ["-drive",
                           f"file={self.cfg.image},if=virtio,format=raw"]

    def _configure_system(self) -> None:
        self.qemu_args += [
            "-machine", self.cfg.machine,
            "-cpu", self.cfg.cpu,
            "-smp", str(self.cfg.smp),
            "-m", self.cfg.ram_size,
            "-nographic",
        ]

    async def _run_qemu(self) -> None:
        logger.getChild("qemu").debug("Run: %s", self.qemu_args)

        self.qemu_process = await asyncio.create_subprocess_exec(
            *self.qemu_args,
            stdin=asyncio.subprocess.DEVNULL,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )

        async for line in self.qemu_process.stdout:
            if not self.is_running:
                break
            logger.getChild("qemu").info(line.decode().strip())

        async for line in self.qemu_process.stderr:
            if not self.is_running:
                break
            logger.getChild("qemu").error(line.decode().strip())

    async def _wait_ssh(self, timeout: int = 120) -> None:
        loop = asyncio.get_running_loop()
        deadline = loop.time() + timeout

        while True:
            try:
                self._conn = await asyncssh.connect(
                    "localhost",
                    username=self.cfg.ssh_user,
                    password=self.cfg.ssh_password,
                    port=self.cfg.ssh_port,
                    known_hosts=None,
                )
                logger.info("SSH ready")
                return
            except (OSError, asyncssh.Error) as e:
                if loop.time() >= deadline:
                    raise RuntimeError(
                        f"SSH not reachable within {timeout}s: {e}") from e
                logger.info("Waiting for ssh (%s)", e)
                await asyncio.sleep(SSH_POLL_INTERVAL)
