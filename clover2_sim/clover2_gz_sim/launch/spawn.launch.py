import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_clover2_gz_sim = get_package_share_directory("clover2_gz_sim")

    # Reading arguments
    world = LaunchConfiguration("world")
    model = LaunchConfiguration("model")
    name = LaunchConfiguration("name")

    # Declare arguments
    world_declare = DeclareLaunchArgument(
        "world",
        description="Gazebo world.",
    )

    model_declare = DeclareLaunchArgument(
        "model",
        description="Select sim model.",
    )

    name_declare = DeclareLaunchArgument(
        "name",
        description="Model name.",
    )

    spawn_cmd = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-world",
            world,
            "-name",
            name,
            "-x",
            "0",
            "-y",
            "0",
            "-z",
            "0.3",
            "-file",
            os.path.join(pkg_clover2_gz_sim, model, "model.sdf"),
        ],
    )

    return LaunchDescription(
        [
            world_declare,
            model_declare,
            name_declare,
            spawn_cmd,
        ]
    )
