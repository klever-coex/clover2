"""Clover2 drone client."""

import atexit
import threading
from collections.abc import Callable
from typing import TypeVar

import rclpy
from rclpy.node import Node

from . import utils
from .clients import (
    CameraClient,
    DisplayClient,
    DronePosition,
    FCUClient,
    LEDClient,
    NavigationTask,
    OffboardClient,
    ThermalCameraClient,
)

T = TypeVar("T")
NAN = float("nan")


class Clover2(Node):
    """Main drone node."""

    def __init__(self, node_name: str = ""):
        """Initialize the drone node."""
        if not node_name or len(node_name) == 0:
            node_name = "client_" + utils.generate_random_string(4)

        rclpy.init()
        Node.__init__(self, node_name)

        self._ros_thread: threading.Thread = threading.Thread(
            target=self._ros_worker, daemon=True
        )

        self._ros_thread.start()
        _ = atexit.register(self._stop)

        self._cached_clients: dict[str, object] = {}
        self._offboard = OffboardClient(self)
        self._fcu = FCUClient(self)

    def _cached_client(self, name: str, factory: Callable[[], T]) -> T | None:
        if name not in self._cached_clients:
            try:
                self._cached_clients[name] = factory()
            except Exception as e:  # noqa: BLE001
                self.get_logger().warning(f"Failed to create client '{name}': {e}")
                return None

        return self._cached_clients[name]

    @property
    def offboard(self) -> OffboardClient:
        return self._offboard

    def set_position(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
    ) -> bool:
        return self.offboard.set_position(frame_id, x, y, z, yaw)

    def navigate(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
        speed: float = 0.5,
    ) -> NavigationTask:
        return self.offboard.navigate(frame_id, x, y, z, yaw, speed)

    def navigate_wait(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
        speed: float = 0.5,
    ) -> bool:
        return self.offboard.navigate_wait(frame_id, x, y, z, yaw, speed)

    def land(self) -> bool:
        return self.offboard.land()

    @property
    def fcu(self) -> FCUClient:
        return self._fcu

    def arm(self) -> bool:
        return self.fcu.arm()

    def disarm(self) -> bool:
        return self.fcu.disarm()

    def is_armed(self) -> bool:
        return self.fcu.is_armed()

    def flight_mode(self) -> str:
        return self.fcu.flight_mode()

    def get_position(self, from_frame: str = "map") -> DronePosition:
        return self.fcu.get_position(from_frame)

    def camera(self, name: str = "main_camera") -> CameraClient | None:
        name_map = {
            "main_camera": "main_camera/camera",
            "front_camera": "front_camera/camera",
        }

        if name in name_map:
            name = name_map[name]

        return self._cached_client(f"{name}", lambda: CameraClient(self, name))

    def thermal_camera(
        self, name: str = "thermal_camera"
    ) -> ThermalCameraClient | None:
        return self._cached_client(f"{name}", lambda: ThermalCameraClient(self, name))

    def led(self, name: str = "led_strip") -> LEDClient | None:
        return self._cached_client(name, lambda: LEDClient(self, name))

    def display(self, name: str = "display") -> DisplayClient | None:
        return self._cached_client(name, lambda: DisplayClient(self, name))

    def _ros_worker(self) -> None:
        while rclpy.ok():
            rclpy.spin(self)

    def _stop(self) -> None:
        self.get_logger().debug(f"Exit callback for {self.get_name()}")

        if rclpy.ok():
            rclpy.shutdown()

            if self._ros_thread is not None:
                self._ros_thread.join()

            self.destroy_node()
