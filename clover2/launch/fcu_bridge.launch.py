#!/usr/bin/env python3

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node

# Set FCU URL based on connection type
fcu_url_mappings = {
    "usb": "/dev/ttyACM0:115200",  # TODO: add udev rules
    "uart": "/dev/ttyAMA0:921600",
    "tcp": "tcp://127.0.0.1:5760",
    "udp": "udp://:14540@localhost:14580",
}


def launch_setup(context, *args, **kwargs):

    # Reading arguments
    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    mavros_params_file = LaunchConfiguration("mavros_params_file")
    fcu_conn = LaunchConfiguration("fcu_conn")

    # Get connection type
    fcu_url = fcu_url_mappings.get(fcu_conn.perform(context), fcu_url_mappings["uart"])

    # Start mavros node
    mavros_node = Node(
        package="mavros",
        executable="mavros_node",
        namespace="mavros",
        parameters=[
            params_file,
            mavros_params_file,
            {
                "gcs_url": "tcp-l://0.0.0.0:5760",
                "use_sim_time": use_sim_time,
                "fcu_url": fcu_url,
            },
        ],
        output="screen",
        arguments=["--ros-args", "--log-level", log_level],
    )

    offboard_server_node = Node(
        package="clover2_fcu_bridge",
        executable="fcu_bridge",
        parameters=[params_file],
        output="screen",
        arguments=["--ros-args", "--log-level", log_level],
    )

    return [mavros_node, offboard_server_node]


def generate_launch_description():
    pkg_clover2 = get_package_share_directory("clover2")

    # Declare arguments
    use_sim_time_declare = DeclareLaunchArgument(
        "use_sim_time",
        default_value="false",
        description="Use simulation clock if true",
    )

    log_level_declare = DeclareLaunchArgument(
        "log_level",
        default_value="info",
        description="Log level for all nodes",
    )

    params_file_declare = DeclareLaunchArgument(
        "params_file",
        default_value=PathJoinSubstitution([pkg_clover2, "param", "default.yaml"]),
        description="Params file",
    )

    mavros_params_file_declare = DeclareLaunchArgument(
        "mavros_params_file",
        default_value=PathJoinSubstitution(
            [pkg_clover2, "params", "mavros_default.yaml"]
        ),
        description="Mavros params file",
    )

    fcu_conn_declare = DeclareLaunchArgument(
        "fcu_conn",
        default_value="udp",
        choices=["usb", "uart", "tcp", "udp"],
        description="Flight controller unit connection type: usb, uart, tcp or udp",
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            mavros_params_file_declare,
            fcu_conn_declare,
            OpaqueFunction(function=launch_setup),
        ]
    )
