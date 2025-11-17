import rclpy
from rclpy.node import Node

import atexit
import threading

from .data import *
from .client import factory
from .client import client_base
from .offboard_helper import OffboardHelper
from . import utils


class Clover2(Node):
    def __init__(self, node_name: str = ""):
        # Initialize ROS2 node
        if not node_name or len(node_name) == 0:
            node_name = "clover2_" + utils.generate_random_string(8)

        self._ros_thread = None

        self._init_ros(node_name)

        self._client = factory.Client(factory.ClientType.MAVROS, node=self)
        self._offboard = OffboardHelper(self, self._client)

        # Stop function on exit
        atexit.register(self._stop)

    def _init_ros(self, node_name: str):

        if not node_name or len(node_name) == 0:
            node_name = "clover2_" + utils.generate_random_string(8)

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
        return self._client.is_armed()

    def get_mode(self) -> FlightMode:
        return self._client.get_mode()

    def set_mode(self, mode: FlightMode) -> bool:
        return self._client.set_mode(mode)

    def get_state(self) -> DroneState:
        return self._client.get_state()

    def get_battery(self) -> Battery:
        return self._client.get_battery()

    def takeoff(self, z: float | int, timeout: float | int = 10.0) -> bool:
        return self._client.takeoff(z, timeout)

    def land(self, timeout: float | int = 10.0) -> bool:
        return self._client.land(timeout)

    def turn_motors_on(self) -> bool:
        future = self._client.turn_motors_on_impl()
        result = utils.wait_future(future, timeout=1.0)
        if result:
            return result.success
        return False

    def turn_motors_off(self) -> bool:
        future = self._client.turn_motors_off_impl()
        return utils.wait_future(future, timeout=1.0).success or False

    def move(
        self,
        x: SetpointValue = None,
        y: SetpointValue = None,
        z: SetpointValue = None,
        yaw: SetpointValue = None,
        speed: SetpointValue = None,
        frame_id: str = "map",
    ):
        setpoint = GoToSetpoint(x, y, z, yaw, speed)
        self._offboard.move(setpoint, frame_id=frame_id)
