import os
import pathlib

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
)


def generate_launch_description():
    pkg_clover2_sim = get_package_share_directory("clover2_sim")

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    sim_type = LaunchConfiguration("sim_type")
    world = LaunchConfiguration("world")
    model = LaunchConfiguration("model")

    # Declare arguments
    use_sim_time_declare = DeclareLaunchArgument(
        "use_sim_time",
        default_value="true",
        description="Use simulation (Gazebo) clock if true",
    )

    log_level_declare = DeclareLaunchArgument(
        "log_level", default_value="info", description="Log level for all nodes"
    )

    params_file_declare = DeclareLaunchArgument(
        "params_file",
        default_value=PathJoinSubstitution([pkg_clover2_sim, "params", "clover5.yaml"]),
        description="Log level for all nodes",
    )

    sim_type_declare = DeclareLaunchArgument(
        "sim_type",
        default_value="gz",
        choices=["gz"],
        description="Type of simulator for now only gz.",
    )

    world_declare = DeclareLaunchArgument(
        "world",
        default_value="simple",
        description="Gazebo world.",
    )

    model_declare = DeclareLaunchArgument(
        "model",
        default_value="x500_mono_cam_down",
        description="Select sim model.",
    )

    sim_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_clover2_sim, "launch", "sim.launch.py"])
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "sim_type": sim_type,
            "world": world,
        }.items(),
    )

    spawn_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_clover2_sim, "launch", "spawn.launch.py"])
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "params_file": params_file,
            "sim_type": sim_type,
            "world": world,
            "model": model,
        }.items(),
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            sim_type_declare,
            world_declare,
            model_declare,
            sim_cmd,
            spawn_cmd,
        ]
    )
