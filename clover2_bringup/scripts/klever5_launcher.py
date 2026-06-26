#!/usr/bin/env python3

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
    for k in data.keys():
        data[k] = str(data[k]).lower() if isinstance(data[k], bool) else data[k]
    return data


def parse_config_from_file(file: pathlib.Path) -> dict:
    with open(file, "r") as f:
        data = yaml.safe_load(f)

    flat = flatten_dict(data)
    return sanitize_dict(flat)


def valid_file(param) -> pathlib.Path:
    path = pathlib.Path(param)
    if not path.exists():
        raise argparse.ArgumentTypeError(f"The file '{param}' does not exist.")
    if not path.is_file():
        raise argparse.ArgumentTypeError(
            f"The path '{param}' is a directory, not a file."
        )
    return path


def parse_args() -> argparse.Namespace:
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
    configuration = parse_config_from_file(args.config)
    for k in configuration.keys():
        print(k)

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
