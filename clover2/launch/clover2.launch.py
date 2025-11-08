#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
    TextSubstitution,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource

from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_clover2 = get_package_share_directory("clover2")

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    fcu_conn = LaunchConfiguration("fcu_conn")
    aruco = LaunchConfiguration("aruco")

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
        "params_file",
        default_value=PathJoinSubstitution([pkg_clover2, "params", "default.yaml"]),
        description="Log level for all nodes",
    )

    fcu_conn_declare = DeclareLaunchArgument(
        "fcu_conn",
        default_value="uart",
        description="Flight controller unit connection type: usb, uart, or tcp",
    )

    aruco_declare = DeclareLaunchArgument(
        "aruco", default_value="true", description="Enable aruco navigation"
    )

    # Start additional launch files
    description_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "description.launch.py"])]
        )
    )

    aruco_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "aruco.launch.py"])]
        ),
        condition=IfCondition(aruco),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "map": "example.yaml",
        }.items(),
    )

    main_camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "camera.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "camera_name": TextSubstitution(text="main_camera"),
            "aruco_detector": aruco,
        }.items(),
    )

    fcu_bridge_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "fcu_bridge.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "fcu_conn": fcu_conn,
        }.items(),
    )

    web_support_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "web_support.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
        }.items(),
    )

    return LaunchDescription(
        [
            # Declare arguments
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            fcu_conn_declare,
            aruco_declare,
            # Launch nodes
            description_cmd,
            aruco_cmd,
            main_camera_cmd,
            fcu_bridge_cmd,
            web_support_cmd,
        ]
    )
