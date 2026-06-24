#!/usr/bin/env python3

import argparse
import sys
from pathlib import Path

import yaml
from ament_index_python.packages import get_package_share_directory
from clover2_bringup import PLATFORM_REGISTRY
from launch import LaunchDescription, LaunchService
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution

pkg_bringup = get_package_share_directory("clover2_bringup")
DEFAULT_PARAMS_FILE = str(Path(pkg_bringup) / "params" / "clover5.yaml")


def parse_arguments() -> argparse.Namespace:

    parser = argparse.ArgumentParser(
        description="Launch Clover2 bringup for a named platform",
    )

    parser.add_argument(
        "config",
        type=Path,
        help="Path to platform configuration YAML file",
    )

    parser.add_argument(
        "--platform",
        "-p",
        default="klever5",
        choices=list(PLATFORM_REGISTRY.keys()),
        help="Platform name (default: klever5)",
    )

    parser.add_argument(
        "--params-file",
        dest="params_files",
        action="append",
        default=None,
        help="Path to ROS parameters YAML file (repeat for multiple files, default: clover5.yaml)",
    )

    return parser.parse_args()


def main(args):
    config_data = yaml.safe_load(args.config.read_text())

    stack_cls, config_cls = PLATFORM_REGISTRY[args.platform]
    config = config_cls.from_dict(config_data)

    use_sim_time = "true" if args.use_sim_time else "false"

    if args.params_files is None:
        args.params_files = [DEFAULT_PARAMS_FILE]

    stack = stack_cls(
        config=config,
        params_files=args.params_files,
        log_level=args.log_level,
        parameters=args.params_files,
        use_sim_time=use_sim_time,
    )

    description = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_bringup, "launch", "description.launch.py"])]
        ),
    )

    web_support = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_bringup, "launch", "web_support.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": args.log_level,
            "params_files": " ".join(args.params_files),
        }.items(),
    )

    ld = LaunchDescription(
        [
            DeclareLaunchArgument(
                "use_sim_time",
                default_value=use_sim_time,
                description="Use simulation (Gazebo) clock if true",
            ),
            DeclareLaunchArgument(
                "log_level",
                default_value=args.log_level,
                description="Log level for all nodes",
            ),
            DeclareLaunchArgument(
                "params_files",
                default_value=" ".join(args.params_files),
                description="Paths to ROS parameters YAML files (space-separated)",
            ),
            stack,
            description,
            web_support,
        ]
    )

    ls = LaunchService(debug=False)
    ls.include_launch_description(ld)
    return ls.run()


if __name__ == "__main__":
    sys.exit(main(parse_arguments()))
