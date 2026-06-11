from pathlib import Path
from typing import List, Optional

from ament_index_python.packages import get_package_share_directory
from clover2.utils import find_file
from launch.action import Action
from launch.actions import LogInfo
from launch.conditions import IfCondition
from launch.launch_context import LaunchContext
from launch.some_substitutions_type import SomeSubstitutionsType
from launch.substitutions import TextSubstitution

from .base import LaunchAction


class LocalizationAction(LaunchAction):
    def __init__(
        self,
        *,
        map_file: SomeSubstitutionsType = TextSubstitution(text="example-1.yaml"),
        map_server: SomeSubstitutionsType = TextSubstitution(text="true"),
        tracker: SomeSubstitutionsType = TextSubstitution(text="true"),
        extra_resource_dirs: Optional[List[Path]] = None,
        **kwargs,
    ) -> None:
        super().__init__(**kwargs)
        pkg_clover2_map = get_package_share_directory("clover2_map")

        self.__map_file = map_file
        self.__map_server = self._coerce_bool(map_server)
        self.__tracker = self._coerce_bool(tracker)
        self.__extra_resource_dirs = extra_resource_dirs or [
            Path(pkg_clover2_map) / "map"
        ]

    def execute(self, context: LaunchContext) -> Optional[List[Action]]:
        actions: List[Action] = []

        map_filename = find_file(
            self._resolve(context, self.__map_file),
            self.__extra_resource_dirs,
        )
        if map_filename is not None:
            actions.append(LogInfo(msg=f"Using map file: {map_filename.as_posix()}"))

        aruco_map_server_node = self._make_node(
            context,
            package="clover2_map",
            executable="server",
            name="map_server",
            parameters=self.parameters
            + [
                {
                    "map": TextSubstitution(
                        text=str(map_filename) if map_filename else ""
                    ),
                    "use_sim_time": self.use_sim_time,
                },
            ],
            condition=IfCondition(self.__map_server),
            respawn_delay=5.0,
        )
        actions.append(aruco_map_server_node)

        tracker_node = self._make_node(
            context,
            package="clover2_aruco",
            executable="tracker",
            name="aruco_tracker",
            parameters=self.parameters + [{"use_sim_time": self.use_sim_time}],
            remappings=[
                ("~/pose_cov", "/mavros/clover2_vio/pose_cov"),
                ("~/map_update", "/map_server/map_update"),
                ("~/get_map", "/map_server/get_map"),
            ],
            condition=IfCondition(self.__tracker),
            respawn_delay=5.0,
        )
        actions.append(tracker_node)

        return actions
