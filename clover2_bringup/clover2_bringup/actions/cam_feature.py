from typing import List, Optional

from launch.action import Action
from launch.frontend import Entity, Parser, expose_action
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.utilities import normalize_to_list_of_substitutions
from launch_ros.actions import LoadComposableNodes
from launch_ros.descriptions import ComposableNode

from .base import LaunchAction
from .resource import ResourceSource


@expose_action("clover2_bringup_cam_feature")
class CamFeatureAction(LaunchAction):
    def __init__(
        self,
        *,
        feature_name: SomeSubstitutionsType = "feat_detector",
        camera: ResourceSource,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self.__feature_name = normalize_to_list_of_substitutions(feature_name)
        self.__camera = camera

    @classmethod
    def parse(cls, entity: Entity, parser: Parser):
        kwargs = super().parse(entity, parser)[1]

        entity.get_attr("feature_name", data_type=str, optional=True)

        return cls, kwargs

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        feature_name = self._resolve(context, self.__feature_name, str)

        cam = self.__camera
        camera_name = self._resolve(context, cam.namespace + ["/"] + cam.name, str)

        remappings = [
            ("~/input/image_raw", f"{camera_name}/camera/image_raw"),
            ("~/input/camera_info", f"{camera_name}/camera/camera_info"),
            ("~/map_update", "/map_server/map_update"),
            ("~/get_map", "/map_server/get_map"),
            ("~/output/markers", "/aruco_tracker/markers"),
        ]

        parameters = self._append_parameters(
            self.parameters,
            [
                {"use_sim_time": self.use_sim_time},
            ],
        )

        return [
            LoadComposableNodes(
                target_container=cam.container_name,
                composable_node_descriptions=[
                    ComposableNode(
                        package="clover2_cam_feature",
                        plugin="clover2::cam_feature::cam_feature",
                        namespace=cam.namespace,
                        name=feature_name,
                        parameters=parameters,
                        remappings=remappings,
                    ),
                ],
            )
        ]
