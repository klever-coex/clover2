from typing import Dict, List, Optional, Union

from launch.action import Action
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitution import Substitution
from launch.substitutions import TextSubstitution
from launch.utilities.type_utils import (
    normalize_to_list_of_substitutions,
    normalize_typed_substitution,
    perform_typed_substitution,
)
from launch_ros.actions import (
    ComposableNodeContainer,
    Node,
)
from launch_ros.descriptions import ComposableNode
from launch_ros.parameters_type import Parameters, SomeParameters
from launch_ros.utilities import normalize_parameters


class LaunchAction(Action):
    def __init__(
        self,
        *,
        log_level: Optional[SomeSubstitutionsType] = None,
        namespace: Optional[SomeSubstitutionsType] = None,
        parameters: Optional[SomeParameters] = None,
        use_sim_time: Optional[SomeSubstitutionsType] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)

        self.__log_level = TextSubstitution(text="info")
        if log_level is not None:
            self.__log_level = normalize_to_list_of_substitutions(log_level)

        self.__namespace = normalize_to_list_of_substitutions(namespace or [])
        self.__parameters = normalize_parameters(parameters or [])

        self.__use_sim_time = TextSubstitution(text="false")
        if use_sim_time is not None:
            self.__use_sim_time = normalize_to_list_of_substitutions(use_sim_time)

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        return []

    @staticmethod
    def _resolve(context, substitution, expected_type=str):
        invar = normalize_typed_substitution(substitution, expected_type)
        return perform_typed_substitution(context, invar, expected_type)

    @staticmethod
    def _coerce_bool(value: Union[bool, str, SomeSubstitutionsType]):
        if isinstance(value, str):
            return normalize_typed_substitution(TextSubstitution(text=value), bool)
        return normalize_typed_substitution(value, bool)

    @staticmethod
    def _coerce_int(value: Union[int, str, SomeSubstitutionsType]):
        if isinstance(value, str):
            return normalize_typed_substitution(TextSubstitution(text=value), int)
        return normalize_typed_substitution(value, int)

    @staticmethod
    def _coerce_float(value: Union[float, str, SomeSubstitutionsType]):
        if isinstance(value, str):
            return normalize_typed_substitution(TextSubstitution(text=value), float)
        return normalize_typed_substitution(value, float)

    @staticmethod
    def _append_parameters(first: Parameters, secound: List[Dict]):
        return first + normalize_parameters(secound)

    def _make_node(
        self,
        context,
        *,
        package: SomeSubstitutionsType,
        executable: SomeSubstitutionsType,
        name: SomeSubstitutionsType = "",
        parameters=None,
        remappings=None,
        condition=None,
        respawn=True,
        respawn_delay=5.0,
    ) -> Node:
        return Node(
            package=package,
            executable=executable,
            name=name,
            namespace=self.__namespace,
            parameters=parameters or [],
            remappings=remappings or [],
            output="screen",
            arguments=["--ros-args", "--log-level", self.__log_level],
            respawn=respawn,
            respawn_delay=respawn_delay,
            condition=condition,
        )

    def _make_composable_node(
        self,
        context,
        *,
        package: SomeSubstitutionsType,
        plugin: SomeSubstitutionsType,
        name: SomeSubstitutionsType = "",
        parameters=None,
        remappings=None,
    ) -> ComposableNode:
        return ComposableNode(
            package=package,
            plugin=plugin,
            name=name,
            namespace=self.__namespace,
            parameters=parameters or [],
            remappings=remappings or [],
        )

    def _make_composable_container(
        self,
        context,
        *,
        name: SomeSubstitutionsType,
        executable: SomeSubstitutionsType = "component_container_mt",
        parameters=None,
    ) -> ComposableNodeContainer:
        return ComposableNodeContainer(
            name=name,
            namespace=self.__namespace,
            package="rclcpp_components",
            executable=executable,
            parameters=parameters or [],
            output="screen",
            arguments=["--ros-args", "--log-level", self.__log_level],
            respawn=True,
            respawn_delay=1.0,
        )

    @property
    def namespace(self) -> List[Substitution]:
        return self.__namespace

    @property
    def parameters(self) -> Parameters:
        return self.__parameters

    @property
    def use_sim_time(self) -> Substitution:
        return self.__use_sim_time

    @property
    def log_level(self) -> Substitution:
        return self.__log_level
