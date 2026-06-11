#!/usr/bin/env python3

from argparse import Namespace
from inspect import Parameter

from ament_index_python.packages import get_package_share_directory
from clover2_bringup.actions import (
    Clover2Stack,
    LibcameraAction,
    LocalizationAction,
    OpticalFlowAction,
)
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
    TextSubstitution,
)


def generate_launch_description():
    pkg_clover2_bringup = get_package_share_directory("clover2_bringup")

    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    fcu_conn = LaunchConfiguration("fcu_conn")

    namespace = None

    main_camera = LibcameraAction(
        camera_id="0",
        camera_name=TextSubstitution(text="main_camera"),
        container_name="main_camera_container",
        log_level=log_level,
        namespace=namespace,
        parameters=[params_file],
        use_sim_time=use_sim_time,
    )

    of = OpticalFlowAction(
        camera=main_camera.as_source(),
        log_level=log_level,
        namespace=namespace,
        optical_flow_name="optical_flow",
        parameters=[params_file],
        use_sim_time=use_sim_time,
    )

    # localization = LocalizationAction(
    #     cameras=[],
    #     params_file=params_file,
    #     use_sim_time=use_sim_time,
    #     log_level=log_level,
    # )

    # stack = Clover2Stack(
    #     camera=camera,
    #     localization=localization,
    # )

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
            # *stack.declare_arguments(),
            DeclareLaunchArgument(
                "fcu_conn",
                default_value="uart",
                description="Flight controller unit connection type: usb, uart, tcp or udp",
            ),
            DeclareLaunchArgument(
                "use_sim_time",
                default_value="false",
                description="Use simulation (Gazebo) clock if true",
            ),
            DeclareLaunchArgument(
                "log_level", default_value="info", description="Log level for all nodes"
            ),
            DeclareLaunchArgument(
                "params_file",
                default_value=PathJoinSubstitution(
                    [pkg_clover2_bringup, "params", "clover5.yaml"]
                ),
                description="Log level for all nodes",
            ),
            # description_cmd,
            # fcu_bridge_cmd,
            # web_support_cmd,
            main_camera,
            of,
        ]
    )
