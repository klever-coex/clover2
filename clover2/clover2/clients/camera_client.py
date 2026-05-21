import threading

import numpy as np
from cv_bridge import CvBridge
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
from sensor_msgs.msg import CameraInfo, Image


class CameraClient:
    def __init__(self, node: Node):
        self._node = node
        self._logger = node.get_logger().get_child("camera")
        self._bridge = CvBridge()

        self._img_subs: dict[str, object] = {}
        self._latest_img: dict[str, Image | None] = {}
        self._img_events: dict[str, threading.Event] = {}

        self._info_subs: dict[str, object] = {}
        self._latest_info: dict[str, CameraInfo | None] = {}
        self._info_events: dict[str, threading.Event] = {}

        self._lock = threading.Lock()

    def get_image(
        self,
        camera_name: str = "main_camera",
        desired_encoding="bgr8",
        timeout: float = 5.0,
    ) -> np.ndarray:
        msg = self.get_image_msg(camera_name, timeout)
        return self._bridge.imgmsg_to_cv2(msg, desired_encoding=desired_encoding)

    def get_image_msg(
        self, camera_name: str = "main_camera", timeout: float = 5.0
    ) -> Image:
        with self._lock:
            if camera_name not in self._img_subs:
                self._create_img_subscription(camera_name)
            event = self._img_events[camera_name]

        if not event.wait(timeout):
            raise TimeoutError(
                f"No image received from '{camera_name}' within {timeout}s"
            )

        with self._lock:
            return self._latest_img[camera_name]

    def get_camera_info(
        self, camera_name: str = "main_camera", timeout: float = 5.0
    ) -> CameraInfo:
        with self._lock:
            if camera_name not in self._info_subs:
                self._create_info_subscription(camera_name)
            event = self._info_events[camera_name]

        if not event.wait(timeout):
            raise TimeoutError(
                f"No camera_info received from '{camera_name}' within {timeout}s"
            )

        with self._lock:
            return self._latest_info[camera_name]

    def _create_img_subscription(self, camera_name: str):
        topic = f"/{camera_name}/camera/image_raw"

        qos = QoSProfile(
            depth=1,
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
        )

        self._latest_img[camera_name] = None
        self._img_events[camera_name] = threading.Event()
        self._img_subs[camera_name] = self._node.create_subscription(
            Image, topic, self._make_img_callback(camera_name), qos
        )
        self._logger.info(f"Subscribed to {topic}")

    def _make_img_callback(self, camera_name: str):
        def callback(msg: Image):
            with self._lock:
                self._latest_img[camera_name] = msg
            self._img_events[camera_name].set()

        return callback

    def _create_info_subscription(self, camera_name: str):
        topic = f"/{camera_name}/camera/camera_info"

        qos = QoSProfile(
            depth=1,
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
        )

        self._latest_info[camera_name] = None
        self._info_events[camera_name] = threading.Event()
        self._info_subs[camera_name] = self._node.create_subscription(
            CameraInfo, topic, self._make_info_callback(camera_name), qos
        )
        self._logger.info(f"Subscribed to {topic}")

    def _make_info_callback(self, camera_name: str):
        def callback(msg: CameraInfo):
            with self._lock:
                self._latest_info[camera_name] = msg
            self._info_events[camera_name].set()

        return callback
