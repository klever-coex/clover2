"""Onboard camera client."""

from collections.abc import Callable

import numpy as np
from cv_bridge import CvBridge
from rclpy.node import Node
from sensor_msgs.msg import CameraInfo, Image

from clover2_common import LatestSubscriber

type ImageCallback = Callable[[Image | np.ndarray], None] | None


class CameraClient:
    """Client for the onboard camera."""

    def __init__(self, node: Node, camera_name: str = "main_camera/camera"):
        """Initialize the camera client."""
        self._bridge = CvBridge()
        self._camera_name = camera_name

        self._img = LatestSubscriber(node, Image, f"{camera_name}/image_raw")
        self._info = LatestSubscriber(node, CameraInfo, f"{camera_name}/camera_info")

    def stream(
        self,
        callback: ImageCallback = None,
        desired_encoding: str = "bgr8",
    ) -> None:
        def on_image(msg: Image) -> None:
            callback(self._bridge.imgmsg_to_cv2(msg, desired_encoding=desired_encoding))

        if callback is not None:
            self.stream_msg(on_image)
        else:
            self.stream_msg(None)

    def stream_msg(self, callback: ImageCallback = None) -> None:
        self._img.stream_data(callback)

    def get_image(
        self, desired_encoding: str = "bgr8", timeout: float = 5.0
    ) -> np.ndarray:
        msg = self.get_image_msg(timeout)
        return self._bridge.imgmsg_to_cv2(msg, desired_encoding=desired_encoding)

    def get_image_msg(self, timeout: float = 5.0) -> Image:
        return self._img.get_data(timeout)

    def get_camera_info(self, timeout: float = 5.0) -> CameraInfo:
        return self._info.get_data(timeout)
