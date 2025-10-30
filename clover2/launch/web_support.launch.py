#!/usr/bin/env python3

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
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
        description='Use simulation clock if true'
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

    # Web video server
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
        respawn_delay=5.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )

    return LaunchDescription([
        use_sim_time_declare,
        log_level_declare,
        params_file_declare,
        web_video_server_cmd,
    ])
