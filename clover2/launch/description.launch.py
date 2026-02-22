#!/usr/bin/env python3

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    camera_transorm_cmd = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="camera_to_base",
        arguments=[
            "--frame-id",
            "base_link",
            "--child-frame-id",
            "main_camera_link",
            "--x",
            "0.05",
            "--y",
            "0.0",
            "--z",
            "-0.07",
            "--roll",
            "0.0",
            "--pitch",
            "3.1415926",
            "--yaw",
            "-1.5707963",
        ],
    )

    return LaunchDescription([camera_transorm_cmd])
