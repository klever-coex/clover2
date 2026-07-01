from pathlib import Path
from typing import Dict, List, Optional, cast

from ament_index_python.packages import get_package_share_directory
from launch.action import Action
from launch.frontend import Entity, Parser, expose_action
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitutions import TextSubstitution
from launch.utilities import ensure_argument_type
from launch.utilities.type_utils import (
    is_substitution,
    normalize_typed_substitution,
    perform_typed_substitution,
)
from launch_ros.actions import Node
from launch_ros.parameters_type import SomeParameters

FCU_URL_MAPPINGS = {
    "usb": "/dev/ttyACM0:115200",
    "uart": "/dev/ttyAMA0:921600",
    "tcp": "tcp://127.0.0.1:5760",
    "udp": "udp://:14540@localhost:14580",
}


@expose_action("fcu_bridge")
class FCUBridgeAction(Action):
    def __init__(
        self,
        *,
        fcu_conn: SomeSubstitutionsType = TextSubstitution(text="udp"),
        protocol: SomeSubstitutionsType = TextSubstitution(text="mavros"),
        mavros_params_file: SomeSubstitutionsType = TextSubstitution(
            text=str(
                Path(get_package_share_directory("clover2"))
                / "params"
                / "mavros_default.yaml"
            )
        ),
        backend: SomeSubstitutionsType = TextSubstitution(text="mavros"),
        namespace: SomeSubstitutionsType = "",
        log_level: SomeSubstitutionsType = "info",
        use_sim_time: SomeSubstitutionsType = "false",
        params_files: Optional[List[str]] = None,
        parameters: Optional[SomeParameters] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)

        self.__fcu_conn = fcu_conn
        self.__protocol = protocol
        self.__mavros_params_file = mavros_params_file
        self.__backend = backend
        self.__namespace = namespace
        self.__log_level = log_level
        self.__use_sim_time = use_sim_time
        self.__params_files = params_files or []

        self.__extra_parameters: List = []
        if parameters is not None:
            if is_substitution(parameters):
                parameters = normalize_typed_substitution(parameters, dict)
            ensure_argument_type(parameters, (list), "parameters", "FCUBridgeAction")
            self.__extra_parameters = cast(list, parameters)

    @classmethod
    def parse(cls, entity: Entity, parser: Parser):
        kwargs: Dict = super().parse(entity, parser)[1]

        for attr in (
            "fcu_conn",
            "protocol",
            "mavros_params_file",
            "backend",
            "namespace",
            "log_level",
        ):
            val = entity.get_attr(attr, data_type=str, optional=True)
            if isinstance(val, str):
                kwargs[attr] = parser.parse_substitution(val)

        use_sim_time_val = entity.get_attr(
            "use_sim_time", data_type=bool, optional=True
        )
        if isinstance(use_sim_time_val, str):
            kwargs["use_sim_time"] = use_sim_time_val.split()

        params_files_val = entity.get_attr("params_files", data_type=str, optional=True)
        if isinstance(params_files_val, str):
            kwargs["params_files"] = params_files_val.split()

        param_entities = entity.get_attr("param", data_type=List[Entity], optional=True)
        if param_entities is not None:
            kwargs["parameters"] = Node.parse_nested_parameters(param_entities, parser)

        return cls, kwargs

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        actions: List[Action] = []

        fcu_conn = perform_typed_substitution(context, self.__fcu_conn, str)
        protocol = perform_typed_substitution(context, self.__protocol, str)
        backend = perform_typed_substitution(context, self.__backend, str)
        namespace = perform_typed_substitution(context, self.__namespace, str)

        if protocol == "mavros":
            fcu_url = FCU_URL_MAPPINGS.get(fcu_conn, FCU_URL_MAPPINGS["uart"])

            mavros_params: List = list(self.__params_files)
            mavros_params.append(self.__mavros_params_file)
            mavros_params.append({"use_sim_time": self.__use_sim_time})
            mavros_params.append(
                {
                    "gcs_url": "tcp-l://0.0.0.0:5760",
                    "fcu_url": fcu_url,
                }
            )
            mavros_params.extend(self.__extra_parameters)

            actions.append(
                Node(
                    package="mavros",
                    executable="mavros_node",
                    namespace=f"{namespace}/mavros",
                    parameters=mavros_params,
                    output="screen",
                    arguments=["--ros-args", "--log-level", "warn"],
                    remappings=[
                        (
                            "/mavros/px4flow/raw/send",
                            "/main_camera/mavros/px4flow/raw/send",
                        ),
                    ],
                    respawn=True,
                    respawn_delay=5.0,
                )
            )

        fcu_bridge_params: List = list(self.__params_files)
        fcu_bridge_params.append({"use_sim_time": self.__use_sim_time})
        fcu_bridge_params.append({"backend": backend})
        fcu_bridge_params.extend(self.__extra_parameters)

        actions.append(
            Node(
                package="clover2_fcu_bridge",
                executable="fcu_bridge",
                name="fcu_bridge",
                namespace=namespace,
                parameters=fcu_bridge_params,
                output="screen",
                arguments=["--ros-args", "--log-level", self.__log_level],
                respawn=True,
                respawn_delay=5.0,
            )
        )

        return actions
