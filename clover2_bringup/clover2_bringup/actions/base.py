from typing import List, Optional, Union

from launch.action import Action
from launch.actions import DeclareLaunchArgument
from launch.frontend import Entity, Parser, expose_action
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitutions import TextSubstitution
from launch.utilities.type_utils import (
    normalize_typed_substitution,
    perform_typed_substitution,
)
from launch_ros.actions import (
    ComposableNodeContainer,
    Node,
)
from launch_ros.descriptions import ComposableNode


@expose_action("clover2_launch_config")
class LaunchAction(Action):
    def __init__(
        self,
        *,
        params_files: list[SomeSubstitutionsType] = [],
        use_sim_time: SomeSubstitutionsType = TextSubstitution(text="false"),
        log_level: SomeSubstitutionsType = TextSubstitution(text="info"),
        namespace: SomeSubstitutionsType = "",
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self._params_files = params_files
        self._use_sim_time = use_sim_time
        self._log_level = log_level
        self._namespace = namespace

    def declare_arguments(self) -> List[DeclareLaunchArgument]:
        return []

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

    @classmethod
    def parse(cls, entity: Entity, parser: Parser):
        kwargs = super().parse(entity, parser)[1]
        return cls, kwargs

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
            namespace=self._namespace,
            parameters=parameters or [],
            remappings=remappings or [],
            output="screen",
            arguments=["--ros-args", "--log-level", self._log_level],
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
            namespace=self._namespace,
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
            namespace=self._namespace,
            package="rclcpp_components",
            executable=executable,
            parameters=parameters or [],
            output="screen",
            arguments=["--ros-args", "--log-level", self._log_level],
            respawn=True,
            respawn_delay=1.0,
        )
