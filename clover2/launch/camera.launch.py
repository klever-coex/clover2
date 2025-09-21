#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, TextSubstitution
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_clover2 = get_package_share_directory('clover2')

    use_sim_time = LaunchConfiguration('use_sim_time')
    log_level = LaunchConfiguration('log_level')
    params_file = LaunchConfiguration('params_file')
    camera_name = LaunchConfiguration('camera_name')

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
        description='Use simulation (Gazebo) clock if true'
    )

    camera_node = ComposableNode(
        package='camera_ros',
        plugin='camera::CameraNode',
        name=camera_name,
        parameters=[
            params_file,
            {
                'use_sim_time': use_sim_time
            }
        ]
    )
    
    aruco_detector_node = ComposableNode(
        package='clover2_aruco',
        plugin='clover2_aruco::detector',
        name=camera_name,
        parameters=[
            params_file,
            {
                'use_sim_time': use_sim_time
            }
        ]
    )

    camera_container_cmd = ComposableNodeContainer(
        name=camera_name + TextSubstitution(text='_container'),
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            camera_node,
            aruco_detector_node,
        ],
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )

    return LaunchDescription([
        use_sim_time_declare,
        log_level_declare,
        params_file_declare,
        camera_name_declare,
        camera_container_cmd,
    ])
