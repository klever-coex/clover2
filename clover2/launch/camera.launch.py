#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch.event_handlers import OnProcessStart
from launch.actions import RegisterEventHandler
from launch_ros.actions import ComposableNodeContainer, LoadComposableNodes
from launch_ros.descriptions import ComposableNode
from ament_index_python.packages import get_package_share_directory


def launch_setup(context, *args, **kwargs):

    use_sim_time = LaunchConfiguration('use_sim_time')
    log_level = LaunchConfiguration('log_level')
    params_file = LaunchConfiguration('params_file')
    camera_name = LaunchConfiguration('camera_name')

    camera_remappings = [
        ('~/image_raw', f'/{camera_name.perform(context)}/image_raw'),
        ('~/camera_info', f'/{camera_name.perform(context)}/camera_info'),
    ]

    map_server_remappings = [
        ('~/map_update', '/map_server/map_update'),
        ('~/get_map', '/map_server/get_map'),
    ]

    launch_nodes = []
    components = [
        ComposableNode(
            package='camera_ros',
            plugin='camera::CameraNode',
            name=camera_name.perform(context),
            parameters=[
                params_file,
                {
                    'use_sim_time': use_sim_time
                }
            ]
        ),
        ComposableNode(
            package='clover2_aruco',
            plugin='clover2_aruco::detector',
            name=camera_name.perform(context) + '_aruco_detector',
            parameters=[
                params_file,
                {
                    'use_sim_time': use_sim_time
                }
            ],
            remappings=camera_remappings + map_server_remappings,
        )]

    camera_container = ComposableNodeContainer(
        name=camera_name.perform(context) + '_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        respawn=True,
        respawn_delay=5.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )

    launch_nodes.append(camera_container)

    launch_nodes.append(RegisterEventHandler(
        OnProcessStart(
            target_action=camera_container,
            on_start=[
                LoadComposableNodes(composable_node_descriptions=components, target_container=camera_container)]
        )))

    return launch_nodes


def generate_launch_description():
    pkg_clover2 = get_package_share_directory('clover2')

    use_sim_time_declare = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) clock if true'
    )

    log_level_declare = DeclareLaunchArgument(
        'log_level',
        default_value='info',
        description='Log level for all nodes'
    )

    params_file_declare = DeclareLaunchArgument(
        'params_file',
        description='Parameters file'
    )

    camera_name_declare = DeclareLaunchArgument(
        'camera_name',
        description='Camera name'
    )

    return LaunchDescription([
        use_sim_time_declare,
        log_level_declare,
        params_file_declare,
        camera_name_declare,
    ] + [OpaqueFunction(function=launch_setup)])
