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

    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    world = LaunchConfiguration("world")
    gui = LaunchConfiguration("gui")

    gz_resource_path = SetEnvironmentVariable(
        name="GZ_SIM_RESOURCE_PATH",
        value=":".join(
            [
                os.path.join(pkg_clover2_gz_sim, "worlds"),
                os.path.join(pkg_clover2_gz_sim, "models"),
            ]
        ),
    )

    gz_server_config_path = SetEnvironmentVariable(
        name="GZ_SIM_SERVER_CONFIG_PATH",
        value=os.path.join(pkg_clover2_gz_sim, "config", "server.config.xml"),
    )

    gz_args = [
        os.path.join(pkg_clover2_gz_sim, "worlds/"),
        world,
        ".sdf",
        " -v 2",
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

    spawn_cmd = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-world",
            "aruco",
            "-name",
            "px4",
            "-x",
            "0",
            "-y",
            "0",
            "-z",
            "0.3",
            "-file",
            "/home/motya/projects/coex/clover2_ws/src/clover2/clover2_sim/clover2_gz_sim/models/x500/model.sdf",
        ],
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            # params_file_declare,
            world_declare,
            gui_declare,
            # gz_resource_path,
            # gz_server_config_path,
            gazebo_cmd,
            spawn_cmd,
        ]
    )


def __headless_rendering(gui):
    cmd = ['"" if "true" == "', gui, '" else "--headless-rendering -s"']
    py_cmd = PythonExpression(cmd)
    return py_cmd
