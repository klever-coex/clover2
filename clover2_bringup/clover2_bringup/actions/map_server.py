from pathlib import Path
from typing import Dict, List, Optional, cast

from ament_index_python.packages import get_package_share_directory
from clover2.config import CLOVER2_RESOURCE_DIR
from clover2.utils import find_file
from launch.action import Action
from launch.actions import LogInfo
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


@expose_action("map_server")
class MapServerAction(Action):
    def __init__(
        self,
        *,
        map_file: SomeSubstitutionsType = TextSubstitution(text="example-1.yaml"),
        resource_paths: Optional[List[SomeSubstitutionsType]] = None,
        namespace: SomeSubstitutionsType = "",
        log_level: SomeSubstitutionsType = "info",
        use_sim_time: SomeSubstitutionsType = "false",
        params_files: Optional[List[str]] = None,
        parameters: Optional[SomeParameters] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        pkg_clover2_map = get_package_share_directory("clover2_map")

        self.__map_file = map_file
        self.__namespace = namespace
        self.__log_level = log_level
        self.__use_sim_time = use_sim_time
        self.__params_files = params_files or []

        self.__resource_paths: List[SomeSubstitutionsType] = (
            resource_paths
            if resource_paths is not None
            else [
                TextSubstitution(text=str(CLOVER2_RESOURCE_DIR / "map")),
                TextSubstitution(text=str(Path(pkg_clover2_map) / "map")),
            ]
        )

        self.__extra_parameters: List = []
        if parameters is not None:
            if is_substitution(parameters):
                parameters = normalize_typed_substitution(parameters, dict)
            ensure_argument_type(parameters, (list), "parameters", "MapServerAction")
            self.__extra_parameters = cast(list, parameters)

    @classmethod
    def parse(cls, entity: Entity, parser: Parser):
        kwargs: Dict = super().parse(entity, parser)[1]

        map_val = entity.get_attr("map", data_type=str, optional=True)
        if isinstance(map_val, str):
            kwargs["map_file"] = parser.parse_substitution(map_val)

        for attr in ("namespace", "log_level", "use_sim_time"):
            val = entity.get_attr(attr, data_type=str, optional=True)
            if isinstance(val, str):
                kwargs[attr] = parser.parse_substitution(val)

        params_files_val = entity.get_attr("params_files", data_type=str, optional=True)
        if isinstance(params_files_val, str):
            kwargs["params_files"] = params_files_val.split()

        param_entities = entity.get_attr("param", data_type=List[Entity], optional=True)
        if param_entities is not None:
            kwargs["parameters"] = Node.parse_nested_parameters(param_entities, parser)

        resource_path_entities = entity.get_attr(
            "resource_path", data_type=List[Entity], optional=True
        )
        if resource_path_entities:
            resource_paths: List[SomeSubstitutionsType] = []
            for rp in resource_path_entities:
                val = rp.get_attr("value", data_type=str, optional=False)
                if isinstance(val, str):
                    resource_paths.append(parser.parse_substitution(val))
            kwargs["resource_paths"] = resource_paths

        return cls, kwargs

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        actions: List[Action] = []

        map_file = perform_typed_substitution(context, self.__map_file, str)
        namespace = perform_typed_substitution(context, self.__namespace, str)

        resource_dirs = [
            Path(perform_typed_substitution(context, rp, str))
            for rp in self.__resource_paths
        ]

        map_filename = find_file(map_file, resource_dirs)
        if map_filename is not None:
            actions.append(LogInfo(msg=f"Using map file: {map_filename.as_posix()}"))

        parameters: List = list(self.__params_files)
        if map_filename is not None:
            parameters.append({"map": str(map_filename)})
        parameters.append({"use_sim_time": self.__use_sim_time})
        parameters.extend(self.__extra_parameters)

        actions.append(
            Node(
                package="clover2_map",
                executable="server",
                name="map_server",
                namespace=namespace,
                parameters=parameters,
                output="screen",
                arguments=["--ros-args", "--log-level", self.__log_level],
                respawn=True,
                respawn_delay=5.0,
            )
        )

        return actions
