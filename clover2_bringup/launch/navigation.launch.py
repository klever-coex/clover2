#!/usr/bin/env python3
from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from clover2.config import CLOVER2_RESOURCE_DIR
from clover2.utils import find_file
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo, OpaqueFunction
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

ARUCO_MAP_FILE = "example-1.yaml"


def launch_setup(context, *args, **kwargs):

    pkg_clover2_map = get_package_share_directory("clover2_map")

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    map = LaunchConfiguration("map")
    aruco_tracker = LaunchConfiguration("aruco_tracker")
    aruco_map_server = LaunchConfiguration("aruco_map_server")

    # Resolve map file path
    map_dirs = [
        CLOVER2_RESOURCE_DIR / "map",
        Path(pkg_clover2_map) / "map",
    ]

    map_filename = find_file(map.perform(context), map_dirs)
    if map_filename is None:
        raise ValueError(
            f"Map file '{map.perform(context)}' not found in any of the expected directories: {map_dirs}"
        )

    print_used_map_cmd = LogInfo(msg=f"Using map file: {map_filename.as_posix()}")

    # Aruco map server
    aruco_map_server_cmd = Node(
        package="clover2_map",
        executable="server",
        name="map_server",
        condition=IfCondition(aruco_map_server),
        parameters=[
            params_file,
            {
                "map": map_filename.as_posix(),
                "use_sim_time": use_sim_time,
            },
        ],
        respawn=True,
        respawn_delay=5.0,
        output="screen",
        arguments=["--ros-args", "--log-level", log_level],
    )

    # Aruco tracker
    tracker_cmd = Node(
        package="clover2_aruco",
        executable="tracker",
        name="aruco_tracker",
        condition=IfCondition(aruco_tracker),
        parameters=[
            params_file,
            {
                "use_sim_time": use_sim_time,
            },
        ],
        respawn=True,
        respawn_delay=5.0,
        output="screen",
        arguments=["--ros-args", "--log-level", log_level],
        remappings=[
            # TODO: remove hardcode
            ("~/markers", "/main_camera/feat_detector/output/markers"),
            ("~/pose_cov", "/mavros/clover2_vio/pose_cov"),
            ("~/map_update", "/map_server/map_update"),
            ("~/get_map", "/map_server/get_map"),
        ],
    )

    return [print_used_map_cmd, aruco_map_server_cmd, tracker_cmd]


def generate_launch_description():
    pkg_clover2 = get_package_share_directory("clover2")

    # Declare arguments
    use_sim_time_declare = DeclareLaunchArgument(
        "use_sim_time",
        default_value="false",
        description="Use simulation clock if true",
    )

    log_level_declare = DeclareLaunchArgument(
        "log_level", default_value="info", description="Log level for all nodes"
    )

    params_file_declare = DeclareLaunchArgument(
        "params_file", description="Parameters file"
    )

    map_declare = DeclareLaunchArgument(
        "map", default_value=ARUCO_MAP_FILE, description="Map name"
    )

    aruco_map_server = DeclareLaunchArgument(
        "aruco_map_server", default_value="true", description="Start aruco map server"
    )

    aruco_tracker = DeclareLaunchArgument(
        "aruco_tracker", default_value="true", description="Start aruco tracker"
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            map_declare,
            aruco_map_server,
            aruco_tracker,
            OpaqueFunction(function=launch_setup),
        ]
    )
