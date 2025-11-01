#!/usr/bin/env python3

from launch import LaunchDescription
from launch_ros.actions import Node

from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    camera_transorm_cmd = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="cmcu06_right_tf",
        arguments=["--frame-id", "base_link", "--child-frame-id", "main_camera_link", "--x", "0.05", "--y", "0.0", "--z", "-0.07", "--roll", "-1.5707963", "--pitch", "0.0", "--yaw", "3.1415926"]
        # arguments=["--frame-id", "base_link", "--child-frame-id", "main_camera_link", "--roll", "0", "--pitch", "3.1415926", "--yaw", "0"]
    )

    return LaunchDescription([
        camera_transorm_cmd
    ])
