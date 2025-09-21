#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, TextSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource

from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_clover2 = get_package_share_directory('clover2')

    # Reading arguments
    use_sim_time = LaunchConfiguration('use_sim_time')
    log_level = LaunchConfiguration('log_level')
    params_file = LaunchConfiguration('params_file')
    mavros_params_file = LaunchConfiguration('mavros_params_file')
    fcu_conn = LaunchConfiguration('fcu_conn')
    serial_device = LaunchConfiguration('serial_device')

    # Declare arguments
    use_sim_time_declare = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation clock if true',
    )

    log_level_declare = DeclareLaunchArgument(
        'log_level',
        default_value='info',
        description='Log level for all nodes',
    )

    params_file_declare = DeclareLaunchArgument(
        'params_file',
        default_value=PathJoinSubstitution([
            pkg_clover2,
            'param',
            'default.yaml'
        ]),
        description='Params file',
    )

    mavros_params_file_declare = DeclareLaunchArgument(
        'mavros_params_file',
        default_value=PathJoinSubstitution([
            pkg_clover2,
            'param',
            'mavros_default.yaml'
        ]),
        description='Mavros params file',
    )

    fcu_conn_declare = DeclareLaunchArgument(
        'fcu_conn',
        default_value='usb',
        choices=['serial', 'serial'],
        description='Flight controller unit connection type',
    )
    
    serial_device_declare = DeclareLaunchArgument(
        'serial_device',
        default_value='/dev/px4fmu',
        description='Flight controller unit usb device',
    )
    
    match (fcu_conn):
        case 'serial':
            pass
        case _:
            pass

    # Start additional launch files
    mavros_cmd = Node(
        package='mavros',
        executable='mavros_node',
        name='mavros',
        parameters=[
            params_file,
            mavros_params_file,
            {
                'use_sim_time': use_sim_time,
                'fcu_url': '',
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
        fcu_conn_declare,
        serial_device_declare,
        mavros_params_file_declare,
        mavros_cmd,
    ])
