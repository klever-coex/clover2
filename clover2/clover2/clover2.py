import atexit
import threading
from collections.abc import Callable
from typing import TypeVar

import rclpy
from rclpy.node import Node

from . import utils
from .clients import CameraClient, DisplayClient, LEDClient, OffboardClient

T = TypeVar("T")


class Clover2(Node):
    def __init__(self, node_name: str = ""):
        if not node_name or len(node_name) == 0:
            node_name = "client_" + utils.generate_random_string(4)

        rclpy.init()
        Node.__init__(self, node_name)

        self._ros_thread: threading.Thread = threading.Thread(
            target=self._ros_worker, daemon=True
        )
        self._ros_thread.start()
        _ = atexit.register(self._stop)

        self._clients: dict[str, object] = {}

        self._offboard: OffboardClient = OffboardClient(self)
        self._camera: CameraClient = CameraClient(self)

    def _cached_client(self, name: str, factory: Callable[[], T]) -> T | None:
        if name not in self._clients:
            try:
                self._clients[name] = factory()
            except Exception:
                self.get_logger().warning(f"Client for '{name}' not found")
                return None

        return self._clients[name]

    @property
    def offboard(self) -> OffboardClient:
        return self._offboard

    @property
    def camera(self) -> CameraClient:
        return self._camera

    def led(self, name: str = "/led_strip") -> LEDClient | None:
        return self._cached_client(name, lambda: LEDClient(self, name))

    def display(self, name: str = "/display") -> DisplayClient | None:
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
