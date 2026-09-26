from dataclasses import dataclass

from clover2_nav_msgs.msg import State
from rclpy.node import Node
from rclpy.time import Time
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf_transformations import euler_from_quaternion

NAN = float("nan")


@dataclass
class DronePosition:
    x: float = NAN
    y: float = NAN
    z: float = NAN
    roll: float = NAN
    pitch: float = NAN
    yaw: float = NAN


class FCUClient:
    def __init__(self, node: Node):
        self._logger = node.get_logger().get_child("fcu")
        self._node = node
        self._state = State()
        self._state_sub = self._node.create_subscription(
            State, "/fcu_bridge/state", self._state_callback, 10
        )
        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, node)

    def is_armed(self) -> bool:
        return self._state.is_armed

    def flight_mode(self) -> str:
        return self._state.mode

    def get_position(self, from_frame: str = "map") -> DronePosition:
        transform = self._tf_buffer.lookup_transform(
            from_frame, "base_link", Time()
        )
        rpy = euler_from_quaternion((
            transform.transform.rotation.x,
            transform.transform.rotation.y,
            transform.transform.rotation.z,
            transform.transform.rotation.w,
        ))
        return DronePosition(
            x=transform.transform.translation.x,
            y=transform.transform.translation.y,
            z=transform.transform.translation.z,
            roll=rpy[0],
            pitch=rpy[1],
            yaw=rpy[2],
        )

    def _state_callback(self, msg: State):
        self._state = msg