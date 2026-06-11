from dataclasses import dataclass
from typing import List, Optional, Union

from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.utilities import normalize_to_list_of_substitutions

from .base import LaunchAction


@dataclass
class ResourceSource:
    name: SomeSubstitutionsType
    container_name: SomeSubstitutionsType
    namespace: SomeSubstitutionsType
    resource_type: str


class ResourceLaunch(LaunchAction):
    def __init__(
        self,
        *,
        node_name: SomeSubstitutionsType,
        container_executable: SomeSubstitutionsType = "component_container_mt",
        container_name: SomeSubstitutionsType = "container",
        create_own_container: Union[bool, SomeSubstitutionsType] = True,
        use_composition: Union[bool, SomeSubstitutionsType] = True,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self.__node_name = node_name
        self.__container_executable = container_executable
        self.__container_name = container_name  # already str
        self.__create_own_container = self._coerce_bool(create_own_container)
        self.__use_composition = self._coerce_bool(use_composition)

        self.__container_name_full = normalize_to_list_of_substitutions(
            self.namespace + ["/", self.__container_name]
        )

    def as_source(self):
        raise NotImplementedError

    def _make_container(self, context, *, name: SomeSubstitutionsType):
        return self._make_composable_container(
            context,
            name=name,
            executable=self.container_executable,
        )

    @property
    def node_name(self) -> SomeSubstitutionsType:
        return self.__node_name

    @property
    def container_executable(self) -> SomeSubstitutionsType:
        return self.__container_executable

    @property
    def container_name(self) -> SomeSubstitutionsType:
        return self.__container_name

    @property
    def create_own_container(self) -> SomeSubstitutionsType:
        return self.__create_own_container

    @property
    def use_composition(self) -> SomeSubstitutionsType:
        return self.__use_composition

    @property
    def container_name_full(self) -> SomeSubstitutionsType:
        return self.__container_name_full
