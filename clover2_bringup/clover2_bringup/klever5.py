from typing import List, Optional

from ament_index_python.packages import get_package_share_directory
from launch.action import Action
from launch.actions import DeclareLaunchArgument
from launch.launch_context import LaunchContext
from launch.substitutions import PathJoinSubstitution, TextSubstitution

from .base import LaunchAction
from .camera import CameraAction
from .localization import LocalizationAction


class Klever5Config:
    pass


class Klever5(LaunchAction):
    def __init__(
        self,
        *,
        camera: Optional[CameraAction] = None,
        localization: Optional[LocalizationAction] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        self._cameras = {
            "main_camera": CameraAction(
                camera_name=TextSubstitution(text="main_camera"),
                params_file=self.params_files,
                use_sim_time=use_sim_time,
                log_level=log_level,
            ),
            "front_camera": CameraAction(
                camera_name=TextSubstitution(text="front_camera"),
                params_file=params_file,
                use_sim_time=use_sim_time,
                log_level=log_level,
            ),
        }
        self._localization = localization
