#!/usr/bin/env python3
from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from clover2.helpers.resource import CLOVER2_RESOURCE_DIR, find_file
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node


def launch_setup(context, *args, **kwargs):

    pkg_clover2_map = get_package_share_directory("clover2_map")

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    map = LaunchConfiguration("map")

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

    map_filename = str(map_filename)

    # Aruco map server
    aruco_map_server_cmd = Node(
        package="clover2_map",
        executable="server",
        name="map_server",
        parameters=[
            params_file,
            {
                "map": map_filename,
                "use_sim_time": use_sim_time,
            },
        ],
        respawn=True,
        respawn_delay=5.0,
        output="screen",
        arguments=["--ros-args", "--log-level", log_level],
    )

    # Aruco tracker
    # tracker_cmd = Node(
    #     package="clover2_aruco",
    #     executable="tracker",
    #     name="aruco_tracker",
    #     parameters=[
    #         params_file,
    #         {
    #             "use_sim_time": use_sim_time,
    #         },
    #     ],
    #     respawn=True,
    #     respawn_delay=5.0,
    #     output="screen",
    #     arguments=["--ros-args", "--log-level", log_level],
    #     remappings=[
    #         # TODO: remove hardcode
    #         ("~/markers", "/main_camera_aruco_detector/markers"),
    #         ("~/pose_cov", "/mavros/vision_pose/pose_cov"),
    #         ("~/map_update", "/map_server/map_update"),
    #         ("~/get_map", "/map_server/get_map"),
    #     ],
    # )

    return [aruco_map_server_cmd]


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
        "map", default_value="example-1.yaml", description="Map name"
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            map_declare,
            OpaqueFunction(function=launch_setup),
        ]
    )
