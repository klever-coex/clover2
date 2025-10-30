#!/usr/bin/env python3

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node

from ament_index_python.packages import get_package_share_directory


def launch_setup(context, *args, **kwargs):

    pkg_clover2_aruco = get_package_share_directory('clover2_aruco')

    # Reading arguments
    use_sim_time = LaunchConfiguration('use_sim_time')
    log_level = LaunchConfiguration('log_level')
    params_file = LaunchConfiguration('params_file')
    map = LaunchConfiguration('map')

    map_filename = PathJoinSubstitution([
        pkg_clover2_aruco,
        'map',
        map.perform(context)
    ])

    # Aruco map server
    aruco_map_server_cmd = Node(
        package='clover2_aruco',
        executable='map_server',
        name='map_server',
        parameters=[
            params_file,
            {
                'map': map_filename,
                'use_sim_time': use_sim_time,
            }
        ],
        respawn=True,
        respawn_delay=5.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level]
    )

    # Aruco tracker
    tracker_cmd = Node(
        package='clover2_aruco',
        executable='tracker',
        name='map_server',
        parameters=[
            params_file,
            {
                'use_sim_time': use_sim_time,
            }
        ],
        respawn=True,
        respawn_delay=5.0,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level],
        remappings=[
            # TODO: remove hardcode
            ('~/markers', '/main_camera_aruco_detector/markers'),
            ('~/pose', '/mavros/vision_pose/pose'),
            ('~/map_update', '/map_server/map_update'),
            ('~/get_map', '/map_server/get_map'),
        ]
    )

    return [aruco_map_server_cmd, tracker_cmd]


def generate_launch_description():
    pkg_clover2 = get_package_share_directory('clover2')

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

    map_declare = DeclareLaunchArgument(
        'map',
        default_value='default.yaml',
        description='Map name'
    )

    return LaunchDescription([
        use_sim_time_declare,
        log_level_declare,
        params_file_declare,
        map_declare,
        OpaqueFunction(function=launch_setup),
    ])
