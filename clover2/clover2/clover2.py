import atexit
import threading

import rclpy
from rclpy.node import Node

from . import utils
from .clients import CameraClient, OffboardClient


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

        self._offboard: OffboardClient = OffboardClient(self)
        self._camera: CameraClient = CameraClient(self)

    def __getattr__(self, name: str):

        for client in [self._offboard, self._camera]:
            ret = getattr(client, name, None)
            if ret:
                return ret

        raise RuntimeError(f"Unknown method {name}")

    @property
    def offboard(self) -> OffboardClient:
        return self._offboard

    @property
    def camera(self) -> CameraClient:
        return self._camera

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
