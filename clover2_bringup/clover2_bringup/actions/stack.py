from pathlib import Path
from typing import List, Optional

from launch.action import Action
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitutions import TextSubstitution
from typing_extensions import Dict

from clover2_bringup.actions import CamFeatureAction

from .base import LaunchAction
from .localization import LocalizationAction
from .resource import ResourceLaunch


class Clover2Stack(LaunchAction):
    def __init__(
        self,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self.__cameras: List = []
        self.__processors: List = []
        self.__localization: Optional[LocalizationAction] = None

    def __camera_constructor(
        self, *, camera_name: SomeSubstitutionsType, **kwargs
    ) -> Optional[ResourceLaunch]:
        return None

    def add_camera(self, camera: ResourceLaunch):
        self.__cameras.append(camera)

    def add_cam_feature_detector(
        self,
        camera: ResourceLaunch,
        feature_name: SomeSubstitutionsType = "feat_detector",
    ):
        self.__processors.append(
            CamFeatureAction(
                camera=camera.as_source(),
                feature_name=feature_name,
                log_level=self.log_level,
                namespace=self.namespace,
                parameters=self.parameters,
                use_sim_time=self.use_sim_time,
            )
        )

    def add_localization(
        self,
        map_file: SomeSubstitutionsType = TextSubstitution(text="example-1.yaml"),
        map_server: SomeSubstitutionsType = TextSubstitution(text="true"),
        tracker: SomeSubstitutionsType = TextSubstitution(text="true"),
        extra_resource_dirs: Optional[List[Path]] = None,
    ):
        self.__localization = LocalizationAction(
            extra_resource_dirs=extra_resource_dirs,
            log_level=self.log_level,
            map_file=map_file,
            map_server=map_server,
            namespace=self.namespace,
            parameters=self.parameters,
            tracker=tracker,
            use_sim_time=self.use_sim_time,
        )

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        actions: List[Action] = []

        for camera in self.__cameras:
            actions.append(camera.execute(context))

        for processor in self.__processors:
            actions.append(processor.execute(context))

        if self.__localization is not None:
            actions.append(self.__localization.execute(context))

        return actions
