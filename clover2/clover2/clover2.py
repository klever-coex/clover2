import atexit
import threading

import rclpy
from rclpy.duration import Duration
from rclpy.node import Node
from bondpy import bondpy

from . import utils
from .clients import CameraClient, OffboardClient

OFFBOARD_BOND_TOPIC = "/fcu_bridge/bond"
OFFBOARD_BOND_ID = "offboard"
OFFBOARD_BOND_CONNECT_TIMEOUT = 10.0
OFFBOARD_BOND_FORM_TIMEOUT = 3.0
OFFBOARD_BOND_HEARTBEAT_PERIOD = 2.0


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

        self._offboard_bond = bondpy.Bond(self, OFFBOARD_BOND_TOPIC, OFFBOARD_BOND_ID)
        self._offboard_bond.set_connect_timeout(OFFBOARD_BOND_CONNECT_TIMEOUT)
        self._offboard_bond.set_heartbeat_period(OFFBOARD_BOND_HEARTBEAT_PERIOD)
        self._offboard_bond.set_formed_callback(self._on_offboard_bond_formed)
        self._offboard_bond.set_broken_callback(self._on_offboard_bond_broken)
        self._offboard_bond.start()

        if not self._offboard_bond.wait_until_formed(
            Duration(seconds=OFFBOARD_BOND_FORM_TIMEOUT)
        ):
            self.get_logger().warn("Offboard bond was not formed within 3 seconds")

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

    def _on_offboard_bond_formed(self) -> None:
        self.get_logger().info("Offboard bond formed")

    def _on_offboard_bond_broken(self) -> None:
        self.get_logger().warn("Offboard bond broken")

    def _stop(self) -> None:
        self.get_logger().debug(f"Exit callback for {self.get_name()}")

        if hasattr(self, "_offboard_bond"):
            self._offboard_bond.break_bond()

        if rclpy.ok():
            rclpy.shutdown()

            if self._ros_thread is not None:
                self._ros_thread.join()

            self.destroy_node()
