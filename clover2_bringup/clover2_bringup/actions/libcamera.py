from typing import List, Optional

from launch.action import Action
from launch.frontend import Entity, Parser, expose_action
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.utilities import normalize_to_list_of_substitutions
from launch_ros.actions import LoadComposableNodes

from .resource import ResourceLaunch, ResourceSource


@expose_action("clover2_bringup_camera")
class LibcameraAction(ResourceLaunch):
    def __init__(
        self,
        *,
        camera_id: str,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self.__camera_id = normalize_to_list_of_substitutions(camera_id)

    def as_source(self) -> ResourceSource:
        return ResourceSource(
            name=self.node_name,
            container_name=self.container_name_full,
            namespace=self.namespace,
            resource_type="camera",
        )

    @classmethod
    def parse(cls, entity: Entity, parser: Parser):
        kwargs = super().parse(entity, parser)[1]

        camera_id = entity.get_attr("camera_id", data_type=str, optional=False)
        camera_name = entity.get_attr("camera_name", data_type=str, optional=False)
        params_file = entity.get_attr("params_file", data_type=str, optional=True)
        container_name = entity.get_attr("container_name", data_type=str, optional=True)
        own_container = entity.get_attr("own_container", data_type=str, optional=True)
        namespace = entity.get_attr("namespace", data_type=str, optional=True)
        use_composition = entity.get_attr(
            "use_composition", data_type=str, optional=True
        )
        log_level = entity.get_attr("log_level", data_type=str, optional=True)

        # TODO:
        return cls, kwargs

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        camera_name = self._resolve(context, self.node_name, str)
        use_composition = self._resolve(context, self.use_composition, bool)
        create_own_container = self._resolve(context, self.create_own_container, bool)

        parameters = self._append_parameters(
            self.parameters,
            [
                {"use_sim_time": self.use_sim_time},
                {"camera": self.__camera_id},
                {"frame_id": camera_name + "_link"},
            ],
        )

        launch_descriptions: List[Action] = []

        if create_own_container:
            launch_descriptions.append(
                self._make_container(context, name=self.container_name)
            )

        if not use_composition:
            launch_descriptions.append(
                self._make_node(
                    context,
                    package="camera_ros",
                    executable="camera_node",
                    name=camera_name,
                    parameters=parameters,
                )
            )
        else:
            launch_descriptions.append(
                LoadComposableNodes(
                    target_container=self.container_name_full,
                    composable_node_descriptions=[
                        self._make_composable_node(
                            context,
                            package="camera_ros",
                            plugin="camera::CameraNode",
                            name=camera_name,
                            parameters=parameters,
                        ),
                    ],
                )
            )

        return launch_descriptions
