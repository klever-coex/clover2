#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, TextSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_clover2 = get_package_share_directory('clover2')
    
    # Reading arguments
    use_sim_time = LaunchConfiguration('use_sim_time')
    log_level = LaunchConfiguration('log_level')
    params_file = LaunchConfiguration('params_file')

    # Declare arguments
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
        default_value=PathJoinSubstitution([
            pkg_clover2,
            'params',
            'default.yaml'
        ]),
        description='Log level for all nodes'
    )

    # Start additional launch files
    main_camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                pkg_clover2,
                'launch',
                'camera.launch.py'
            ])
        ]),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'log_level': log_level,
            'params_file': params_file,
            'camera_name': TextSubstitution(text='main_camera'),
        }.items()
    )

    mavros_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                pkg_clover2,
                'launch',
                'mavros.launch.py'
            ])
        ]),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'log_level': log_level,
            'params_file': params_file,
        }.items()
    )
    
    web_video_server_cmd = Node(
        package='web_video_server',
        executable='web_video_server',
        name='web_video_server',
        parameters=[
            params_file,
            {
                'use_sim_time': use_sim_time,
            }
        ],
        respawn=True,
        respawn_delay=1.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )
    
    aruco_map_server_cmd = Node(
        package='clover2_aruco',
        executable='map_server',
        name='map_server',
        parameters=[
            params_file,
            {
                'use_sim_time': use_sim_time,
                'map': get_package_share_directory('clover2_aruco') + '/map/example.yaml'
            }
        ],
        respawn=True,
        respawn_delay=1.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )

    return LaunchDescription([
        use_sim_time_declare,
        log_level_declare,
        params_file_declare,
        main_camera_cmd,
        # mavros_cmd,
        aruco_map_server_cmd,
        web_video_server_cmd,
    ])
