#!/usr/bin/env python3

import os
from asyncio import Condition

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
    TextSubstitution,
)

ENABLE_NAVIGATION = True
ENABLE_OPTICAL_FLOW = True
ENABLE_FRONT_CAMERA = False


def generate_launch_description():
    pkg_clover2 = get_package_share_directory("clover2")

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    fcu_conn = LaunchConfiguration("fcu_conn")
    navigation = LaunchConfiguration("navigation")
    optical_flow = LaunchConfiguration("optical_flow")
    simulation = LaunchConfiguration("simulation")
    front_camera = LaunchConfiguration("front_camera")

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
        default_value=PathJoinSubstitution([pkg_clover2, "params", "clover5.yaml"]),
        description="Log level for all nodes",
    )

    fcu_conn_declare = DeclareLaunchArgument(
        "fcu_conn",
        default_value="uart",
        description="Flight controller unit connection type: usb, uart, tcp or udp",
    )

    navigation_declare = DeclareLaunchArgument(
        "navigation",
        default_value="true" if ENABLE_NAVIGATION else "false",
        description="Enable navigation",
    )

    optical_flow_declare = DeclareLaunchArgument(
        "optical_flow",
        default_value="true" if ENABLE_OPTICAL_FLOW else "false",
        description="Enable optical flow",
    )

    simulation_declare = DeclareLaunchArgument(
        "simulation",
        default_value="false",
        description="Start simulation mode",
    )

    front_camera_declare = DeclareLaunchArgument(
        "front_camera",
        default_value="true" if ENABLE_FRONT_CAMERA else "false",
        description="Start front camera",
    )

    # Start additional launch files
    description_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "description.launch.py"])]
        )
    )

    navigation_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "navigation.launch.py"])]
        ),
        condition=IfCondition(navigation),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "map": "simulation.yaml",
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
            "feature_detector": navigation,
            "optical_flow": optical_flow,
            "simulation": simulation,
        }.items(),
    )

    front_camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "camera.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "camera_name": TextSubstitution(text="front_camera"),
            "feature_detector": TextSubstitution(text="false"),
            "optical_flow": TextSubstitution(text="false"),
            "simulation": simulation,
        }.items(),
        condition=IfCondition(front_camera),
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
            navigation_declare,
            optical_flow_declare,
            simulation_declare,
            front_camera_declare,
            # Launch nodes
            description_cmd,
            navigation_cmd,
            main_camera_cmd,
            front_camera_cmd,
            fcu_bridge_cmd,
            web_support_cmd,
        ]
    )
