import threading

import numpy as np
from cv_bridge import CvBridge
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
from sensor_msgs.msg import Image


class ThermalCameraClient:
    def __init__(self, node: Node):
        self._node = node
        self._logger = node.get_logger().get_child("thermal_camera")
        self._bridge = CvBridge()
        self._subscriptions: dict[str, object] = {}
        self._latest: dict[str, Image] = {}
        self._events: dict[str, threading.Event] = {}
        self._lock = threading.Lock()

    def get_temperature(
        self, camera_name: str = "thermal_camera", timeout: float = 5.0
    ) -> np.ndarray:
        msg = self.get_temperature_msg(camera_name, timeout)
        return self._bridge.imgmsg_to_cv2(msg)

    def get_temperature_msg(
        self, camera_name: str = "thermal_camera", timeout: float = 5.0
    ) -> Image:
        topic = f"/{camera_name.strip('/')}/temperature"
        with self._lock:
            if topic not in self._subscriptions:
                self._create_subscription(topic)
            event = self._events[topic]

        if not event.wait(timeout):
            raise TimeoutError(
                f"No temperature received from '{topic}' within {timeout}s"
            )

        with self._lock:
            return self._latest[topic]

    def _create_subscription(self, topic: str) -> None:
        event = threading.Event()
        self._events[topic] = event

        def callback(msg: Image) -> None:
            with self._lock:
                self._latest[topic] = msg
                event.set()

        qos = QoSProfile(
            depth=1,
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
        )
        self._subscriptions[topic] = self._node.create_subscription(
            Image, topic, callback, qos
        )
        self._logger.info(f"Subscribed to {topic}")
