#!/usr/bin/env python3

import sys
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

ENABLE_NAVIGATION = True
ENABLE_FRONT_CAMERA = False

pkg_clover2_bringup = get_package_share_directory("clover2_bringup")


def generate_launch_description():

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_files = LaunchConfiguration("params_files")
    fcu_conn = LaunchConfiguration("fcu_conn")
    navigation = LaunchConfiguration("navigation")
    map = LaunchConfiguration("map")
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

    params_files_declare = DeclareLaunchArgument(
        "params_files",
        default_value=PathJoinSubstitution(
            [pkg_clover2_bringup, "params", "clover5.yaml"]
        ),
        description="Parameters files (space-separated)",
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
            [
                PathJoinSubstitution(
                    [pkg_clover2_bringup, "launch", "description.launch.py"]
                )
            ]
        ),
    )

    # Launch aruco nodes
    map_declare = DeclareLaunchArgument(
        "map", default_value="example-1.yaml", description="Map file"
    )

    navigation_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [pkg_clover2_bringup, "launch", "navigation.launch.py"]
                )
            ]
        ),
        condition=IfCondition(navigation),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_files": params_files,
            "map": map,
        }.items(),
    )

    main_camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2_bringup, "launch", "camera.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_files": params_files,
            "camera_name": TextSubstitution(text="main_camera"),
            "simulation": simulation,
        }.items(),
    )

    front_camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [PathJoinSubstitution([pkg_clover2_bringup, "launch", "camera.launch.py"])]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_files": params_files,
            "camera_name": TextSubstitution(text="front_camera"),
            "simulation": simulation,
        }.items(),
        condition=IfCondition(front_camera),
    )

    fcu_bridge_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [pkg_clover2_bringup, "launch", "fcu_bridge.launch.py"]
                )
            ]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_files": params_files,
            "fcu_conn": fcu_conn,
        }.items(),
    )

    web_support_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [pkg_clover2_bringup, "launch", "web_support.launch.py"]
                )
            ]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_files": params_files,
        }.items(),
    )

    return LaunchDescription(
        [
            # Declare arguments
            use_sim_time_declare,
            log_level_declare,
            params_files_declare,
            fcu_conn_declare,
            navigation_declare,
            map_declare,
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


def main(argv=sys.argv[1:]):
    ls = LaunchService(argv=argv)
    ld = generate_launch_description()

    print(LaunchIntrospector().format_launch_description(ld))

    ls.include_launch_description(ld)

    return ls.run()


if __name__ == "__main__":
    sys.exit(main())
