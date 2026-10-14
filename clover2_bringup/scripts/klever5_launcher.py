#!/usr/bin/env python3
"""Klever5 launcher script."""

import argparse
import pathlib

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription, LaunchService
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import AnyLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution

pkg_bringup = get_package_share_directory("clover2_bringup")


def flatten_dict(d: dict, sep: str = ".") -> dict:
    """Flatten a nested dict."""
    flat_dict = {}
    stack = [(d, "")]
    while stack:
        current_dict, parent_key = stack.pop()
        for k, v in current_dict.items():
            new_key = f"{parent_key}{sep}{k}" if parent_key else k
            if isinstance(v, dict):
                stack.append((v, new_key))
            else:
                flat_dict[new_key] = v

    return flat_dict


def sanitize_dict(data: dict) -> dict:
    """Stringify boolean values in a dict."""
    for k, v in data.items():
        data[k] = str(v).lower() if isinstance(v, bool) else v
    return data


def parse_config_from_file(file: pathlib.Path) -> dict:
    """Parse a YAML configuration file."""
    with open(file, "r") as f:
        data = yaml.safe_load(f)

    flat = flatten_dict(data)
    return sanitize_dict(flat)


def valid_file(param) -> pathlib.Path:
    """Check that the path is an existing file."""
    path = pathlib.Path(param)
    if not path.exists():
        raise argparse.ArgumentTypeError(f"The file '{param}' does not exist.")
    if not path.is_file():
        raise argparse.ArgumentTypeError(
            f"The path '{param}' is a directory, not a file."
        )
    return path


def parse_args() -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="Launch clover2 bringup",
    )

    parser.add_argument(
        "--config",
        "-c",
        required=True,
        type=valid_file,
        help="Path to yaml configuration for clover2 launch",
    )

    return parser.parse_args()


def main(args: argparse.Namespace):
    """Launch clover2 bringup."""
    configuration = parse_config_from_file(args.config)

    ld = LaunchDescription(
        [
            IncludeLaunchDescription(
                AnyLaunchDescriptionSource(
                    PathJoinSubstitution([pkg_bringup, "launch", "klever5.launch.xml"])
                ),
                launch_arguments=configuration.items(),
            )
        ]
    )

    ls = LaunchService(debug=False)
    ls.include_launch_description(ld)
    return ls.run()


if __name__ == "__main__":
    main(parse_args())
