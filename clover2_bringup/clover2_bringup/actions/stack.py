from typing import List, Optional

from ament_index_python.packages import get_package_share_directory
from launch.action import Action
from launch.actions import DeclareLaunchArgument
from launch.launch_context import LaunchContext
from launch.substitutions import PathJoinSubstitution

from .base import LaunchAction
from .camera import CameraAction
from .localization import LocalizationAction


class Clover2Stack(LaunchAction):
    def __init__(
        self,
        *,
        camera: Optional[CameraAction] = None,
        localization: Optional[LocalizationAction] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self._camera = camera
        self._localization = localization

    def declare_arguments(self) -> List[DeclareLaunchArgument]:
        args: List[DeclareLaunchArgument] = [
            DeclareLaunchArgument(
                "use_sim_time",
                default_value="false",
                description="Use simulation clock if true",
            ),
            DeclareLaunchArgument(
                "log_level",
                default_value="info",
                description="Log level for all nodes",
            ),
            DeclareLaunchArgument(
                "params_file",
                default_value=PathJoinSubstitution(
                    [
                        get_package_share_directory("clover2"),
                        "params",
                        "clover5.yaml",
                    ]
                ),
                description="Parameters file",
            ),
        ]
        if self._camera is not None:
            args.extend(self._camera.declare_arguments())
        if self._localization is not None:
            args.extend(self._localization.declare_arguments())
        return args

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        actions: List[Action] = []
        cameras = []
        if self._camera is not None:
            camera_actions = self._camera.execute(context)
            if camera_actions:
                actions.extend(camera_actions)
            cameras.append(self._camera.as_source(context))
        if self._localization is not None:
            if not self._localization._cameras and cameras:
                self._localization._cameras = cameras
            loc_actions = self._localization.execute(context)
            if loc_actions:
                actions.extend(loc_actions)
        return actions
