import os
import pathlib
import shutil

import click

AMENT_INDEX_PACKAGE = "share/ament_index/resource_index/packages"


def exec_ros(argv: list[str]) -> None:
    ros2 = shutil.which("ros2")
    if ros2 is None:
        raise click.ClickException(
            "'ros2' not found on PATH; source the ROS 2 environment first "
            "(e.g. 'source /opt/ros/jazzy/setup.bash')")

    os.execvp(ros2, ["ros2", *argv])


def package_share(package: str) -> pathlib.Path | None:
    prefixes = os.environ.get("AMENT_PREFIX_PATH", "")
    for entry in filter(None, prefixes.split(os.pathsep)):
        prefix = pathlib.Path(entry)
        if (prefix / AMENT_INDEX_PACKAGE / package).is_file():
            return prefix / "share" / package

    return None
