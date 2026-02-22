#!/usr/bin/env python3

import os
import sys
from enum import EnumType
from operator import ge

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription, LaunchIntrospector, LaunchService
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
    TextSubstitution,
)

pkg_clover2 = get_package_share_directory("clover2")


def fcu_bridge_declare():
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")

    fcu_bridge = LaunchConfiguration("fcu_bridge")
    fcu_protocol = LaunchConfiguration("fcu_protocol")
    fcu_conn = LaunchConfiguration("fcu_conn")

    fcu_bridge_declare = DeclareLaunchArgument(
        "fcu_bridge",
        default_value="true",
        description="Enable flight controller unit bridge",
    )

    fcu_protocol_declare = DeclareLaunchArgument(
        "fcu_protocol",
        default_value="mavlink",
        choices=["mavlink"],
        description="Flight controller unit protocol: mavlink, udp or tcp",
    )

    fcu_conn_declare = DeclareLaunchArgument(
        "fcu_conn",
        default_value="uart",
        description="Flight controller unit connection type: usb, uart, tcp or udp",
    )

    fcu_bridge_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "fcu_bridge.launch.py"])]
        ),
        condition=IfCondition(fcu_bridge),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "fcu_conn": fcu_conn,
            "fcu_protocol": fcu_protocol,
        }.items(),
    )

    return [
        fcu_bridge_declare,
        fcu_protocol_declare,
        fcu_conn_declare,
        fcu_bridge_cmd,
    ]


def camera_declare(name: str):
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")

    enable_camera = LaunchConfiguration(name + "_enable")
    aruco = LaunchConfiguration(name + "_aruco")

    enable_camera_declare = DeclareLaunchArgument(
        name + "_enable",
        default_value="false",
        description=f"Enable camera for {name}",
    )

    aruco_declare = DeclareLaunchArgument(
        name + "_aruco",
        default_value="true",
        description=f"Enable aruco navigation for {name} camera",
    )

    camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "camera.launch.py"])]
        ),
        condition=IfCondition(enable_camera),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "camera_name": TextSubstitution(text=name),
            "aruco_detector": aruco,
        }.items(),
    )

    return [
        enable_camera_declare,
        aruco_declare,
        camera_cmd,
    ]


def generate_launch_description():

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")

    map = LaunchConfiguration("map")
    aruco_tracker = LaunchConfiguration("aruco_tracker")
    aruco_map_server = LaunchConfiguration("aruco_map_server")
    optical_flow = LaunchConfiguration("optical_flow")

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

    optical_flow_declare = DeclareLaunchArgument(
        "optical_flow", default_value="true", description="Enable optical flow"
    )

    # Start additional launch files
    description_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "description.launch.py"])]
        ),
    )

    # Launch aruco nodes
    map_declare = DeclareLaunchArgument(
        "map", default_value="example-1.yaml", description="Map file"
    )

    aruco_map_server_declare = DeclareLaunchArgument(
        "aruco_map_server", default_value="true", description="Enable aruco map server"
    )

    aruco_tracker_declare = DeclareLaunchArgument(
        "aruco_tracker", default_value="true", description="Enable aruco tracker"
    )

    aruco_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2, "launch", "aruco.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "map": map,
            "tracker": aruco_tracker,
            "map_server": aruco_map_server,
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
        fcu_bridge_declare()
        + camera_declare("near_camera")
        + camera_declare("front_camera")
        + [
            # Declare arguments
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            aruco_map_server_declare,
            aruco_tracker_declare,
            optical_flow_declare,
            map_declare,
            # Launch nodes
            description_cmd,
            aruco_cmd,
            web_support_cmd,
        ]
    )


def main(argv=sys.argv[1:]):
    ls = LaunchService(argv=argv)
    ld = generate_launch_description()

    print(LaunchIntrospector().format_launch_description(ld))

    ls.include_launch_description(ld)

    # return ls.run()


if __name__ == "__main__":
    sys.exit(main())
