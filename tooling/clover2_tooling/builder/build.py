import asyncio
import logging
import os
import pathlib
import shlex

from clover2_tooling.builder.components.chroot import Chroot, ChrootConfig
from clover2_tooling.builder.components.qemu import Qemu, QemuConfig
from clover2_tooling.builder.settings import BuilderSettings

logger = logging.getLogger(__name__)

VM_REPO_DIR = "/opt/clover2/ws/src/clover2"
REPO_CLONE_URL = "https://github.com/klever-coex/clover2.git"
RUNNER_PATH = f"{VM_REPO_DIR}/tooling/clover2_tooling/builder/image-setup.sh"
KERNEL_PATH = pathlib.Path(__file__).parent / "assets" / "kernel-qemu-raspi4"
SSH_USER, SSH_PASSWORD = "pi", "raspberry"


def default_build_extras(project_dir: pathlib.Path) -> pathlib.Path:
    return pathlib.Path(os.environ.get(
        "BUILD_EXPTRAS_DIR", project_dir / "build-extras"))


async def inject_assets(image: pathlib.Path, tars: list[pathlib.Path],
                        sudo: bool = True) -> None:
    chroot_cfg = ChrootConfig(
        image,
        {
            1: "",      # mount to /
            0: "boot",  # mount to /boot
        },
        with_sudo=sudo,
    )

    async with Chroot(chroot_cfg) as chroot:
        assets = pathlib.Path(__file__).parent / "assets"
        await chroot.copy_to(assets / "01-nopasswd", "etc/sudoers.d")
        await chroot.copy_to(assets / "user-data", "boot")
        for tar in tars:
            await chroot.copy_to(tar, "root/")


async def _clone_project(qemu: Qemu, git_hash: str) -> None:
    await qemu.execute("""sudo mkdir -p /opt/clover2/ws/src &&
sudo chown -R $USER:$USER /opt/clover2""")

    logger.info("Clone project")
    await qemu.execute(f"git clone {REPO_CLONE_URL} {VM_REPO_DIR}", check=False)
    await qemu.execute(f"cd {VM_REPO_DIR} && git fetch origin && git switch --detach {git_hash}")


async def _run_stage(qemu: Qemu, env: str, stage: str) -> None:
    await qemu.execute(
        f"cd {VM_REPO_DIR} && {env} bash {RUNNER_PATH} --stages {stage}".strip())


async def _load_build_extras(qemu: Qemu, project_dir: str) -> None:
    build_extras = default_build_extras(project_dir)
    if not build_extras.is_dir():
        raise RuntimeError(
            f"Build extras dir '{build_extras}' is missing "
            "(docker tars / debs for the image)")

    await qemu.copy_to(build_extras, "/tmp/clover2-build-extras")


async def provision(settings: BuilderSettings, payload: dict, image: pathlib.Path,
                    stages_args: str = "") -> None:
    qemu_cfg = QemuConfig(
        image=image,
        ssh_user=SSH_USER,
        ssh_password=SSH_PASSWORD,
        smp=12,
        extra_args=(
            "-append", "console=ttyAMA0,115200 root=/dev/vda2 rw",
            "-kernel", str(KERNEL_PATH),
        ),
    )

    async with Qemu(qemu_cfg) as qemu:
        env = (
            f"REGISTRY={shlex.quote(settings.registry)} "
            f"CLOVER2_VERSION={shlex.quote(payload['version'])} "
            f"CLOVER2_GIT_HASH={shlex.quote(payload['git_hash'])} "
        )
        
        await _clone_project(qemu, payload["git_hash"])

        async with asyncio.TaskGroup() as tg:
            tg.create_task(_load_build_extras(qemu, settings.project_dir))
            tg.create_task(_run_stage(qemu, env, '"00-common,10-ros,11-ros-extra,30-docker"'))
            tg.create_task(_run_stage(qemu, env, '"20-camera"'))
            tg.create_task(_run_stage(qemu, env, '"40-hardware"'))
            tg.create_task(_run_stage(qemu, env, '"50-netplan"'))
            tg.create_task(_run_stage(qemu, env, '"60-user"'))
            tg.create_task(_run_stage(qemu, env, '"71-copy-clover2-files"'))

        logger.info("Run over stages")
        await qemu.execute(
            f"cd {VM_REPO_DIR} && {env} bash {RUNNER_PATH} {stages_args}".strip())


def build(settings: BuilderSettings, payload: dict, image: pathlib.Path,
          tars: list[pathlib.Path], stages_args: str = "") -> None:
    asyncio.run(_build(settings, payload, image, tars, stages_args))


async def _build(settings, payload, image, tars, stages_args) -> None:
    await inject_assets(image, tars)
    await provision(settings, payload, image, stages_args)
