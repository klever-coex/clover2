from dataclasses import dataclass, field
from typing import List, Optional

# from launch.action import Action
# from launch.actions import DeclareLaunchArgument
# from launch.launch_context import LaunchContext
# from launch.substitutions import PathJoinSubstitution, TextSubstitution
from clover2_bringup.actions import (
    Clover2Stack,
    LaunchAction,
    LibcameraAction,
    LocalizationAction,
)
from clover2_bringup.configs import (
    CameraConfig,
    FCUBridgeConfig,
    LocalizationConfig,
    OpticalFlowConfig,
)


@dataclass
class Klever5Config:
    main_camera: CameraConfig = field(default_factory=CameraConfig)
    front_camera: CameraConfig = field(default_factory=CameraConfig)
    optical_flow: OpticalFlowConfig = field(default_factory=OpticalFlowConfig)
    fcu_bridge: FCUBridgeConfig = field(default_factory=FCUBridgeConfig)
    localization: LocalizationConfig = field(default_factory=LocalizationConfig)


class Klever5(Clover2Stack):
    def __init__(
        self,
        *,
        config: Klever5Config,
        camera: Optional[LibcameraAction] = None,
        localization: Optional[LocalizationAction] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)

        if config.main_camera.enable:
            main_camera = self.__create_camera("main_camera", "0")
            self.add_camera(main_camera)

            if config.main_camera.feature_detector:
                self.add_cam_feature_detector(
                    camera=main_camera,
                    feature_name="feat_detector",
                )

        if config.front_camera.enable:
            front_camera = self.__create_camera("front_camera", "1")
            self.add_camera(front_camera)

        if config.localization.enable:
            self.add_localization(
                map_server=config.localization.map_server.enable,
                tracker=config.localization.tracker.enable,
                map_file=config.localization.map_server.map_filename,
            )

    def __create_camera(self, camera_name: str, camera_id: str):
        return LibcameraAction(
            camera_id=camera_id,
            node_name=camera_name,
            namespace=self.namespace,
            parameters=self.parameters,
            use_composition=True,
            log_level=self.log_level,
            create_own_container=True,
            container_name=f"{camera_name}_container",
        )
