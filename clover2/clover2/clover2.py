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

        self.offboard = Offboard(self)

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
        return self.offboard.is_armed()

    def arm(self) -> bool:
        """Alias of arm()"""
        return self.offboard.arm()

    def disarm(self) -> bool:
        """Alias of disarm()"""
        return self.offboard.disarm()

    def land(self) -> bool:
        """Alias of land()"""
        return self.offboard.land()

    def get_mode(self) -> str:
        """Alias of get_mode()"""
        return self.offboard.get_mode()

    # def set_mode(self, mode: FlightMode) -> bool:
    #     return self._client.set_mode(mode)

    # def get_state(self) -> DroneState:
    #     return self._client.get_state()

    # def get_battery(self) -> Battery:
    #     return self._client.get_battery()

    # def takeoff(self, z: float | int, timeout: float | int = 10.0) -> bool:
    #     return self._client.takeoff(z, timeout)

    # def turn_motors_on(self) -> bool:
    #     future = self._client.turn_motors_on_impl()
    #     result = utils.wait_future(future, timeout=1.0)
    #     if result:
    #         return result.success
    #     return False

    # def turn_motors_off(self) -> bool:
    #     future = self._client.turn_motors_off_impl()
    #     return utils.wait_future(future, timeout=1.0).success or False

    # def move(
    #     self,
    #     x: SetpointValue = None,
    #     y: SetpointValue = None,
    #     z: SetpointValue = None,
    #     yaw: SetpointValue = None,
    #     speed: SetpointValue = 0.3,
    #     frame_id: str = "map",
    # ):
    #     pos = Point(x=x, y=y, z=z)
    #     vel = Vector3(x=speed, y=speed, z=speed)
    #     self._offboard.move(pos=pos, vel=vel, yaw=yaw, yaw_rate=0.1, frame_id=frame_id)
