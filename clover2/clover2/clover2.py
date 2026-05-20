import atexit
import threading

import rclpy
from rclpy.node import Node

from . import utils
from .offboard import Offboard


class Clover2(Node):
    def __init__(self, node_name: str = ""):
        # Initialize ROS2 node
        if not node_name or len(node_name) == 0:
            node_name = "client_" + utils.generate_random_string(4)

        self._ros_thread = None

        self._init_ros(node_name)

        self._offboard = Offboard(self)

        # Stop function on exit
        atexit.register(self._stop)

    def _init_ros(self, node_name: str):

        rclpy.init()
        Node.__init__(self, node_name)

        self._ros_thread = threading.Thread(target=self._ros_worker, daemon=True)
        self._ros_thread.start()

        self.get_logger().info("ROS processor started")

    def _ros_worker(self):
        while rclpy.ok():
            rclpy.spin(self)

    def _stop(self):
        self.get_logger().debug(f"Exit callback for {self.get_name()}")

        if rclpy.ok():
            rclpy.shutdown()

            if self._ros_thread is not None:
                self._ros_thread.join()

            self.destroy_node()

    def is_armed(self) -> bool:
        """Alias of is_armed()"""
        return self._offboard.is_armed()

    def arm(self) -> bool:
        """Alias of arm()"""
        return self._offboard.arm()

    def disarm(self) -> bool:
        """Alias of disarm()"""
        return self._offboard.disarm()

    def land(self) -> bool:
        """Alias of land()"""
        return self._offboard.land()

    def flight_mode(self) -> str:
        """Alias of get_mode()"""
        return self._offboard.flight_mode()
