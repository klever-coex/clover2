from typing import List, Optional

from launch.action import Action
from launch.actions import DeclareLaunchArgument
from launch.conditions import UnlessCondition
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitutions import TextSubstitution
from launch_ros.actions import LoadComposableNodes
from launch_ros.descriptions import ComposableNode

from .base import LaunchAction
from .resource import ResourceLaunch, ResourceSource


class CameraAction(ResourceLaunch):
    def __init__(
        self,
        *,
        camera_name: SomeSubstitutionsType,
        camera_id: str,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self._camera_id = camera_id
        self._camera_name = camera_name

    def declare_arguments(self) -> List[DeclareLaunchArgument]:
        return [
            DeclareLaunchArgument(
                "camera_name",
                description="Camera name (e.g. main_camera)",
            ),
            DeclareLaunchArgument(
                "simulation",
                default_value="false",
                description="Start in simulation mode",
            ),
        ]

    def as_source(self, context: LaunchContext) -> ResourceSource:
        name = self._resolve(context, self._camera_name)
        return ResourceSource(
            name=name,
            container_name=f"/{name}/container",
            namespace=name,
            resource_type="camera",
        )

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        camera_name = self._resolve(context, self._camera_name)

        container = self._make_container(
            context, name=TextSubstitution(text="container")
        )

        camera_node = LoadComposableNodes(
            target_container=container,
            composable_node_descriptions=[
                ComposableNode(
                    package="camera_ros",
                    plugin="camera::CameraNode",
                    namespace=camera_name,
                    name="camera",
                    parameters=self._params_files
                    + [
                        {"use_sim_time": self._use_sim_time},
                        {"camera": self._camera_id},
                        {"frame_id": camera_name + "_link"},
                    ],
                )
            ],
        )

        return [container, camera_node]
