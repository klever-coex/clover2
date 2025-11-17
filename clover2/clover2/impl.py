import rclpy
from rclpy.node import Node

from . import utils
import threading


class Clover2(Node):
    def __init__(self, node_name=None, autostart=True):
        if not node_name:
            node_name = "clover2_" + utils.generate_random_string(8)

        super().__init__(node_name)

        self.thread = threading.Thread(target=self.ros_worker)
        if autostart:
            self.open()

    def ok(self):
        return self.is_ros_active and rclpy.ok()

    def open(self):
        self.is_ros_active = True
        self.thread.start()

    def close(self):
        self.is_ros_active = False
        self.thread.join()

        self.destroy_node()

    def ros_worker(self):
        while self.is_ros_active and rclpy.ok():
            rclpy.spin_once(self, timeout_sec=0.5)
