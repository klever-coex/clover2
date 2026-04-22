import os
import pathlib

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    SetEnvironmentVariable,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    EnvironmentVariable,
    LaunchConfiguration,
    PathJoinSubstitution,
    PythonExpression,
)
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_clover2_gz_sim = get_package_share_directory("clover2_gz_sim")

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    world = LaunchConfiguration("world")
    gui = LaunchConfiguration("gui")

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
        description="Log level for all nodes",
    )

    world_declare = DeclareLaunchArgument(
        "world",
        default_value="simple",
        description="Gazebo world.",
    )

    gui_declare = DeclareLaunchArgument(
        "gui",
        default_value="true",
        choices=["true", "false"],
        description='Set to "false" to run headless.',
    )

    gz_args = [
        os.path.join(pkg_clover2_gz_sim, "worlds/"),
        world,
        ".sdf",
        " -v 1",
        " -r",
        __headless_rendering(gui),
    ]

    gazebo_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution(
                [FindPackageShare("ros_gz_sim"), "launch", "gz_sim.launch.py"]
            )
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "log_level": log_level,
            "gz_args": gz_args,
        }.items(),
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            # params_file_declare,
            world_declare,
            gui_declare,
            gazebo_cmd,
        ]
    )


def __headless_rendering(gui):
    cmd = ['"" if "true" == "', gui, '" else "--headless-rendering -s"']
    py_cmd = PythonExpression(cmd)
    return py_cmd
