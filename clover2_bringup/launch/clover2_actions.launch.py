#!/usr/bin/env python3

from ament_index_python.packages import get_package_share_directory
from clover2_bringup.actions import CameraAction, Clover2Stack, LocalizationAction
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    LaunchActionuration,
    PathJoinSubstitution,
    TextSubstitution,
)


def generate_launch_description():
    pkg_clover2_bringup = get_package_share_directory("clover2_bringup")

    use_sim_time = LaunchActionuration("use_sim_time")
    log_level = LaunchActionuration("log_level")
    params_file = LaunchActionuration("params_file")
    fcu_conn = LaunchActionuration("fcu_conn")

    camera = CameraAction(
        camera_name=TextSubstitution(text="main_camera"),
        params_files=[params_file],
        use_sim_time=use_sim_time,
        log_level=log_level,
        camera_id=TextSubstitution(text="0"),
    )

    localization = LocalizationAction(
        cameras=[],
        params_file=params_file,
        use_sim_time=use_sim_time,
        log_level=log_level,
    )

    stack = Clover2Stack(
        camera=camera,
        localization=localization,
    )

    description_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [pkg_clover2_bringup, "launch", "description.launch.py"]
                )
            ]
        ),
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
            "params_file": params_file,
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
            "params_file": params_file,
        }.items(),
    )

    return LaunchDescription(
        [
            *stack.declare_arguments(),
            DeclareLaunchArgument(
                "fcu_conn",
                default_value="uart",
                description="Flight controller unit connection type: usb, uart, tcp or udp",
            ),
            description_cmd,
            fcu_bridge_cmd,
            web_support_cmd,
            stack,
        ]
    )
