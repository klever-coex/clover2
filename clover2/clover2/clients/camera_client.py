import threading
from collections.abc import Callable

import numpy as np
from cv_bridge import CvBridge
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
from sensor_msgs.msg import CameraInfo, Image


class CameraClient:
    def __init__(self, node: Node, camera_name: str = "main_camera"):
        self._node = node
        self._logger = node.get_logger().get_child("camera")
        self._bridge = CvBridge()
        self._camera_name = camera_name

        self._img_sub = None
        self._latest_img: Image | None = None
        self._img_event = threading.Event()
        self._stream_callbacks: list[Callable[[Image], None]] = []

        self._info_sub = None
        self._latest_info: CameraInfo | None = None
        self._info_event = threading.Event()

        self._lock = threading.Lock()

    def stream(self, callback: Callable[[Image], None]) -> None:
        with self._lock:
            self._ensure_img_subscription()
            self._stream_callbacks.append(callback)

    def get_image(
        self, desired_encoding: str = "bgr8", timeout: float = 5.0
    ) -> np.ndarray:
        msg = self.get_image_msg(timeout)
        return self._bridge.imgmsg_to_cv2(msg, desired_encoding=desired_encoding)

    def get_image_msg(self, timeout: float = 5.0) -> Image:
        with self._lock:
            self._ensure_img_subscription()

        if not self._img_event.wait(timeout):
            raise TimeoutError(
                f"No image received from '{self._camera_name}' within {timeout}s"
            )

        with self._lock:
            assert self._latest_img is not None
            return self._latest_img

    def get_camera_info(self, timeout: float = 5.0) -> CameraInfo:
        with self._lock:
            self._ensure_info_subscription()

        if not self._info_event.wait(timeout):
            raise TimeoutError(
                f"No camera_info received from '{self._camera_name}' within {timeout}s"
            )

        with self._lock:
            assert self._latest_info is not None
            return self._latest_info

    def _ensure_img_subscription(self) -> None:
        if self._img_sub is not None:
            return

        topic = f"/{self._camera_name}/camera/image_raw"
        self._img_sub = self._node.create_subscription(
            Image, topic, self._img_callback, self._sensor_qos()
        )
        self._logger.info(f"Subscribed to {topic}")

    def _ensure_info_subscription(self) -> None:
        if self._info_sub is not None:
            return

        topic = f"/{self._camera_name}/camera/camera_info"
        self._info_sub = self._node.create_subscription(
            CameraInfo, topic, self._info_callback, self._sensor_qos()
        )
        self._logger.info(f"Subscribed to {topic}")

    def _img_callback(self, msg: Image) -> None:
        with self._lock:
            self._latest_img = msg
            callbacks = list(self._stream_callbacks)
            self._img_event.set()

        for callback in callbacks:
            try:
                callback(msg)
            except Exception:
                self._logger.error("Camera stream callback failed")

    def _info_callback(self, msg: CameraInfo) -> None:
        with self._lock:
            self._latest_info = msg
            self._info_event.set()

    @staticmethod
    def _sensor_qos() -> QoSProfile:
        return QoSProfile(
            depth=1,
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
        )