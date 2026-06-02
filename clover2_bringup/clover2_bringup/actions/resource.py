from dataclasses import dataclass
from typing import List, Optional

from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch_ros.actions import ComposableNodeContainer

from .base import LaunchAction


@dataclass
class ResourceSource:
    name: str
    container_name: str
    namespace: str
    resource_type: str


class ResourceLaunch(LaunchAction):
    def __init__(
        self,
        *,
        container_executable: SomeSubstitutionsType = "component_container_mt",
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self._container_executable = container_executable

    def as_source(self, context: LaunchContext):
        raise NotImplementedError

    def _make_container(self, context, *, name: SomeSubstitutionsType):
        return self._make_composable_container(
            context,
            name=name,
            executable=self._container_executable,
        )
