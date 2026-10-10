from dataclasses import dataclass

from clover2_common import wait_future
from clover2_nav_msgs.msg import State
from clover2_nav_msgs.srv import ArmDisarm
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
        self._arm_disarm_client = self._node.create_client(
            ArmDisarm, "/fcu_bridge/arm_disarm"
        )
        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, node)

    def arm_disarm(self, arm: bool) -> bool:
        req = ArmDisarm.Request()
        req.arm = arm
        return self.__wait_service_call(self._arm_disarm_client, req)

    def arm(self) -> bool:
        return self.arm_disarm(True)

    def disarm(self) -> bool:
        return self.arm_disarm(False)

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

    def __wait_service_call(self, srv, request, timeout=1.0) -> bool:
        result = wait_future(srv.call_async(request), timeout=timeout)
        if not result:
            self._logger.error("Service did not respond")
            return False
        if not result.success:
            self._logger.error(f"`{srv.service_name}`: {result.message}")
        return result.success
