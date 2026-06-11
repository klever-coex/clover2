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


@expose_action("clover2_bringup_optical_flow")
class OpticalFlowAction(LaunchAction):
    def __init__(
        self,
        *,
        camera: ResourceSource,
        optical_flow_name: SomeSubstitutionsType = "optical_flow",
        roi: int = 256,
        calc_flow_gyro: bool = False,
        flow_gyro_default: float = float("nan"),
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self.__optical_flow_name = normalize_to_list_of_substitutions(optical_flow_name)
        self.__camera = camera
        self.__roi = self._coerce_int(roi)
        self.__calc_flow_gyro = self._coerce_bool(calc_flow_gyro)
        self.__flow_gyro_default = self._coerce_float(flow_gyro_default)

    @classmethod
    def parse(cls, entity: Entity, parser: Parser):
        kwargs = super().parse(entity, parser)[1]

        entity.get_attr("optical_flow_name", data_type=str, optional=True)
        entity.get_attr("roi", data_type=int, optional=True)
        entity.get_attr("calc_flow_gyro", data_type=str, optional=True)
        entity.get_attr("flow_gyro_default", data_type=float, optional=True)

        return cls, kwargs

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        optical_flow_name = self._resolve(context, self.__optical_flow_name, str)
        roi = self._resolve(context, self.__roi, int)
        calc_flow_gyro = self._resolve(context, self.__calc_flow_gyro, bool)
        flow_gyro_default = self._resolve(context, self.__flow_gyro_default, float)

        cam = self.__camera
        camera_name = self._resolve(context, cam.namespace + ["/"] + cam.name, str)

        camera_remappings = [
            ("~/input/image_raw", f"{camera_name}/camera/image_raw"),
            ("~/input/camera_info", f"{camera_name}/camera/camera_info"),
        ]

        parameters = self._append_parameters(
            self.parameters,
            [
                {"use_sim_time": self.use_sim_time},
                {"roi": roi},
                {"calc_flow_gyro": calc_flow_gyro},
                {"flow_gyro_default": flow_gyro_default},
            ],
        )

        return [
            LoadComposableNodes(
                target_container=cam.container_name,
                composable_node_descriptions=[
                    ComposableNode(
                        package="clover2_optical_flow",
                        plugin="clover2::optical_flow::optical_flow",
                        namespace=cam.namespace,
                        name=optical_flow_name,
                        parameters=parameters,
                        remappings=camera_remappings,
                    ),
                ],
            )
        ]
