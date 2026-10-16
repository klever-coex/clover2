"""Latest-message subscriber helper."""

import threading
from collections.abc import Callable
from typing import Generic, TypeVar

from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy

MsgT = TypeVar("MsgT")


def sensor_qos() -> QoSProfile:
    """Return the sensor QoS profile."""
    return QoSProfile(
        depth=1,
        reliability=QoSReliabilityPolicy.BEST_EFFORT,
    )


SENSOR_QOS = sensor_qos()


class LatestSubscriber(Generic[MsgT]):
    """Subscription with latest-message cache and optional stream callback."""

    def __init__(
        self,
        node: Node,
        msg_type: type[MsgT],
        topic: str,
        qos: QoSProfile = SENSOR_QOS,
    ):
        """Initialize the subscriber."""
        self._logger = node.get_logger().get_child("latest_subscriber")
        self._latest: MsgT | None = None
        self._event = threading.Event()
        self._stream_callback: Callable[[MsgT], None] | None = None
        self._lock = threading.Lock()

        self._sub = node.create_subscription(msg_type, topic, self._on_msg, qos)

    def get_data(self, timeout: float = 5.0) -> MsgT:
        if not self._event.wait(timeout):
            raise TimeoutError(f"No message received on topic within {timeout}s")

        with self._lock:
            assert self._latest is not None
            return self._latest

    def stream_data(self, callback: Callable[[MsgT], None] | None) -> None:
        with self._lock:
            self._stream_callback = callback

    def _on_msg(self, msg: MsgT) -> None:
        with self._lock:
            self._latest = msg
            callback = self._stream_callback
            self._event.set()

        if callback is None:
            return

        try:
            callback(msg)
        except Exception:  # noqa: BLE001
            self._logger.error("Stream callback failed")
