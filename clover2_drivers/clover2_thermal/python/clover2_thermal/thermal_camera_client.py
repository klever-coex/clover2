"""Thermal camera client."""

from collections.abc import Callable

import numpy as np
from cv_bridge import CvBridge
from rclpy.node import Node
from sensor_msgs.msg import Image

from clover2_common import LatestSubscriber

type ThermalCallback = Callable[[Image | np.ndarray], None] | None


class ThermalCameraClient:
    """Client for the thermal camera."""

    def __init__(self, node: Node, thermal_name: str = "thermal_camera"):
        """Initialize the thermal camera client."""
        self._bridge = CvBridge()
        self._thermal_name = thermal_name

        self._temp = LatestSubscriber(node, Image, f"{thermal_name}/temperature")

    def stream_temperature(
        self,
        callback: ThermalCallback = None,
        desired_encoding: str = "bgr8",
    ) -> None:
        def on_image(msg: Image) -> None:
            callback(self._bridge.imgmsg_to_cv2(msg, desired_encoding=desired_encoding))

        if callback is not None:
            self._temp.stream_data(on_image)
        else:
            self._temp.stream_data(None)

    def get_temperature(self, timeout: float = 5.0) -> np.ndarray:
        msg = self.get_temperature_msg(timeout)
        return self._bridge.imgmsg_to_cv2(msg)

    def get_temperature_msg(self, timeout: float = 5.0) -> Image:
        return self._temp.get_data(timeout)
