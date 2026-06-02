from pathlib import Path
from typing import List, Optional

from ament_index_python.packages import get_package_share_directory
from clover2.config import CLOVER2_RESOURCE_DIR
from clover2.utils import find_file
from launch.action import Action
from launch.actions import DeclareLaunchArgument, LogInfo
from launch.conditions import IfCondition
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitutions import TextSubstitution
from launch_ros.actions import LoadComposableNodes
from launch_ros.descriptions import ComposableNode

from .base import LaunchAction
from .resource import ResourceSource


class LocalizationAction(LaunchAction):
    def __init__(
        self,
        *,
        cameras: List[ResourceSource],
        map_file: SomeSubstitutionsType = TextSubstitution(text="example-1.yaml"),
        aruco_map_server: SomeSubstitutionsType = TextSubstitution(text="true"),
        aruco_tracker: SomeSubstitutionsType = TextSubstitution(text="true"),
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self._cameras = cameras
        self._map_file = map_file
        self._aruco_map_server = self._coerce_bool(aruco_map_server)
        self._aruco_tracker = self._coerce_bool(aruco_tracker)

    def declare_arguments(self) -> List[DeclareLaunchArgument]:
        return [
            DeclareLaunchArgument(
                "map",
                default_value="example-1.yaml",
                description="Map file name",
            ),
            DeclareLaunchArgument(
                "aruco_map_server",
                default_value="true",
                description="Enable ArUco map server",
            ),
            DeclareLaunchArgument(
                "aruco_tracker",
                default_value="true",
                description="Enable ArUco tracker",
            ),
        ]

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        actions: List[Action] = []
        pkg_clover2_map = get_package_share_directory("clover2_map")

        for cam in self._cameras:
            camera_remappings = [
                ("~/input/image_raw", f"/{cam.name}/camera/image_raw"),
                ("~/input/camera_info", f"/{cam.name}/camera/camera_info"),
            ]
            map_server_remappings = [
                ("~/map_update", "/map_server/map_update"),
                ("~/get_map", "/map_server/get_map"),
            ]

            feature_detector = LoadComposableNodes(
                target_container=cam.container_name,
                composable_node_descriptions=[
                    ComposableNode(
                        package="clover2_cam_feature",
                        plugin="clover2::cam_feature::cam_feature",
                        namespace=cam.namespace,
                        name="feat_detector",
                        parameters=self._params_files
                        + [
                            {"use_sim_time": self._use_sim_time},
                        ],
                        remappings=(
                            camera_remappings
                            + map_server_remappings
                            + [("~/markers", "/aruco_tracker/markers")]
                        ),
                    )
                ],
            )
            actions.append(feature_detector)

            optical_flow = LoadComposableNodes(
                target_container=cam.container_name,
                composable_node_descriptions=[
                    ComposableNode(
                        package="clover2_optical_flow",
                        plugin="clover2::optical_flow::optical_flow",
                        namespace=cam.namespace,
                        name="optical_flow",
                        parameters=self._params_files
                        + [
                            {"use_sim_time": self._use_sim_time},
                        ],
                        remappings=camera_remappings,
                    )
                ],
            )
            actions.append(optical_flow)

        map_filename = find_file(
            self._resolve(context, self._map_file),
            [
                CLOVER2_RESOURCE_DIR / "map",
                Path(pkg_clover2_map) / "map",
            ],
        )
        if map_filename is not None:
            actions.append(LogInfo(msg=f"Using map file: {map_filename.as_posix()}"))

        aruco_map_server_node = self._make_node(
            context,
            package="clover2_map",
            executable="server",
            name="map_server",
            parameters=self._params_files
            + [
                {
                    "map": TextSubstitution(
                        text=str(map_filename) if map_filename else ""
                    ),
                    "use_sim_time": self._use_sim_time,
                },
            ],
            condition=IfCondition(self._aruco_map_server),
            respawn_delay=5.0,
        )
        actions.append(aruco_map_server_node)

        tracker_node = self._make_node(
            context,
            package="clover2_aruco",
            executable="tracker",
            name="aruco_tracker",
            parameters=self._params_files + [{"use_sim_time": self._use_sim_time}],
            remappings=[
                ("~/markers", "/main_camera/feat_detector/output/markers"),
                ("~/pose_cov", "/mavros/clover2_vio/pose_cov"),
                ("~/map_update", "/map_server/map_update"),
                ("~/get_map", "/map_server/get_map"),
            ],
            condition=IfCondition(self._aruco_tracker),
            respawn_delay=5.0,
        )
        actions.append(tracker_node)

        return actions
