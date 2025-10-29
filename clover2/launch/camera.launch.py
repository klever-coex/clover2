#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.conditions import UnlessCondition
from launch.substitutions import LaunchConfiguration, PythonExpression
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
    reuse_container = LaunchConfiguration('reuse_container')
    container_name = LaunchConfiguration('container_name')

    map_server_remappings = [
        ('~/map_update', '/map_server/map_update'),
        ('~/get_map', '/map_server/get_map'),
    ]

    launch_nodes = []
    
    camera_container = ComposableNodeContainer(
        condition=UnlessCondition(PythonExpression(["'", reuse_container, "' == 'true'"])),
        name=container_name,
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        respawn=True,
        respawn_delay=1.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )

    launch_nodes.append(camera_container)    
    
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
            remappings=map_server_remappings,
        )]

    launch_nodes.append(RegisterEventHandler(
        OnProcessStart(
            target_action=camera_container,
            on_start=[
                LoadComposableNodes(
                    composable_node_descriptions=components,
                    target_container=container_name
                )]
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
    
    reuse_container_declare = DeclareLaunchArgument(
        'reuse_container',
        default_value='true'
    )
    
    container_name_declare = DeclareLaunchArgument(
        'container_name',
        default_value='camera_container'
    )

    return LaunchDescription([
        use_sim_time_declare,
        log_level_declare,
        params_file_declare,
        camera_name_declare,
        reuse_container_declare,
        container_name_declare
    ] + [OpaqueFunction(function=launch_setup)])
