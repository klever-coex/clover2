#!/usr/bin/env python3

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    GroupAction,
    OpaqueFunction,
    SetEnvironmentVariable,
)
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import (
    ComposableNodeContainer,
    LoadComposableNodes,
)
from launch_ros.descriptions import ComposableNode


def make_gz_bridge_topics(camera_name: str):
    cam_image = f"{camera_name}_camera_image"
    cam_info = f"{camera_name}_camera_info"
    return [
        {"bridge_names": [cam_image, cam_info, "odom"]},
        # Camera image
        {f"bridges.{cam_image}.ros_topic_name": "~/image_raw"},
        {
            f"bridges.{cam_image}.gz_topic_name": "/world/clover2_aruco/model/px4/link/camera_link/sensor/imager/image"
        },
        {f"bridges.{cam_image}.ros_type_name": "sensor_msgs/msg/Image"},
        {f"bridges.{cam_image}.gz_type_name": "gz.msgs.Image"},
        {f"bridges.{cam_image}.direction": "GZ_TO_ROS"},
        {f"bridges.{cam_image}.frame_id": camera_name},
        # Camera info
        {f"bridges.{cam_info}.ros_topic_name": "~/camera_info"},
        {
            f"bridges.{cam_info}.gz_topic_name": "/world/clover2_aruco/model/px4/link/camera_link/sensor/imager/camera_info"
        },
        {f"bridges.{cam_info}.ros_type_name": "sensor_msgs/msg/CameraInfo"},
        {f"bridges.{cam_info}.gz_type_name": "gz.msgs.CameraInfo"},
        {f"bridges.{cam_info}.direction": "GZ_TO_ROS"},
        {f"bridges.{cam_info}.frame_id": camera_name},
        {"bridges.odom.ros_topic_name": "/mavros/distance_sensor/rangefinder"},
        {
            "bridges.odom.gz_topic_name": "/world/clover2_aruco/model/px4/link/rangefinder_link/sensor/rangefinder/scan"
        },
        {"bridges.odom.ros_type_name": "sensor_msgs/msg/Range"},
        {"bridges.odom.gz_type_name": "gz.msgs.LaserScan"},
        {"bridges.odom.direction": "GZ_TO_ROS"},
        {"bridges.odom.frame_id": "camera_link"},
    ]


def launch_setup(context, *args, **kwargs):

    use_sim_time = LaunchConfiguration("use_sim_time")
    log_level = LaunchConfiguration("log_level")
    params_file = LaunchConfiguration("params_file")
    camera_name = LaunchConfiguration("camera_name")
    feature_detector = LaunchConfiguration("feature_detector")
    optical_flow = LaunchConfiguration("optical_flow")
    simulation = LaunchConfiguration("simulation")

    camera_remappings = [
        ("~/input/image_raw", f"/{camera_name.perform(context)}/camera/image_raw"),
        ("~/input/camera_info", f"/{camera_name.perform(context)}/camera/camera_info"),
    ]

    map_server_remappings = [
        ("~/map_update", "/map_server/map_update"),
        ("~/get_map", "/map_server/get_map"),
    ]

    camera_container = ComposableNodeContainer(
        name="container",
        namespace=camera_name.perform(context),
        package="rclcpp_components",
        executable="component_container_mt",
        respawn=True,
        respawn_delay=1.0,
        output="screen",
        arguments=["--ros-args", "--log-level", log_level],
    )

    camera_component = LoadComposableNodes(
        condition=UnlessCondition(simulation),
        target_container=camera_container,
        composable_node_descriptions=[
            ComposableNode(
                package="camera_ros",
                plugin="camera::CameraNode",
                namespace=camera_name.perform(context),
                name="camera",
                parameters=[
                    params_file,
                    {"use_sim_time": use_sim_time},
                    {"frame_id": camera_name.perform(context)},
                ],
            )
        ],
    )

    sim_camera_component = LoadComposableNodes(
        condition=IfCondition(simulation),
        target_container=camera_container,
        composable_node_descriptions=[
            ComposableNode(
                package="ros_gz_bridge",
                plugin="ros_gz_bridge::RosGzBridge",
                namespace=camera_name.perform(context),
                name="camera",
                parameters=[
                    params_file,
                    {"use_sim_time": use_sim_time},
                ]
                + make_gz_bridge_topics(camera_name.perform(context)),
            )
        ],
    )

    feature_detector_component = LoadComposableNodes(
        condition=IfCondition(feature_detector),
        target_container=camera_container,
        composable_node_descriptions=[
            ComposableNode(
                package="clover2_cam_feature",
                plugin="clover2::cam_feature::cam_feature",
                namespace=camera_name.perform(context),
                name="feat_detector",
                parameters=[params_file, {"use_sim_time": use_sim_time}],
                remappings=camera_remappings + map_server_remappings,
            )
        ],
    )

    optical_flow_component = LoadComposableNodes(
        condition=IfCondition(optical_flow),
        target_container=camera_container,
        composable_node_descriptions=[
            ComposableNode(
                package="clover2_optical_flow",
                plugin="clover2::optical_flow::optical_flow",
                namespace=camera_name.perform(context),
                name="optical_flow",
                parameters=[params_file, {"use_sim_time": use_sim_time}],
                remappings=camera_remappings,
            )
        ],
    )

    return [
        GroupAction(
            actions=[
                camera_container,
                camera_component,
                sim_camera_component,
                feature_detector_component,
                optical_flow_component,
            ]
        ),
    ]


def generate_launch_description():
    pkg_clover2 = get_package_share_directory("clover2")

    use_sim_time_declare = DeclareLaunchArgument(
        "use_sim_time",
        default_value="false",
        description="Use simulation (Gazebo) clock if true",
    )

    log_level_declare = DeclareLaunchArgument(
        "log_level", default_value="info", description="Log level for all nodes"
    )

    params_file_declare = DeclareLaunchArgument(
        "params_file", description="Parameters file"
    )

    camera_name_declare = DeclareLaunchArgument(
        "camera_name", description="Camera name"
    )

    feature_detector_declare = DeclareLaunchArgument(
        "feature_detector",
        default_value="false",
        description="Enable feature detection on this camera",
    )

    optical_flow_declare = DeclareLaunchArgument(
        "optical_flow",
        default_value="false",
        description="Enable optical flow calculation on this camera",
    )

    simulation_declare = DeclareLaunchArgument(
        "simulation",
        default_value="false",
        description="Start simulation mode",
    )

    return LaunchDescription(
        [
            use_sim_time_declare,
            log_level_declare,
            params_file_declare,
            camera_name_declare,
            feature_detector_declare,
            optical_flow_declare,
            simulation_declare,
            OpaqueFunction(function=launch_setup),
        ]
    )
