import uuid as uuid_lib
from dataclasses import dataclass

from bondpy import bondpy
from clover2_nav_msgs.action import NavigateAsync
from clover2_nav_msgs.msg import State
from clover2_nav_msgs.srv import ArmDisarm, Land, Navigate, SetPosition
from geometry_msgs.msg import Pose
from rclpy.action import ActionClient
from rclpy.node import Node
from rclpy.time import Time
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf_transformations import quaternion_from_euler, euler_from_quaternion
from unique_identifier_msgs.msg import UUID

from ..utils import ActionHelper, ActionStatus, wait_future

NAN = float("nan")
NAVIGATE_BOND_TOPIC = "/fcu_bridge/bond"
NAVIGATE_BOND_CONNECT_TIMEOUT = 2.0
NAVIGATE_BOND_HEARTBEAT_PERIOD = 0.2
NAVIGATE_BOND_HEARTBEAT_TIMEOUT = 1.0


@dataclass
class DronePosition:
    x: float = NAN
    y: float = NAN
    z: float = NAN
    roll: float = NAN
    pitch: float = NAN
    yaw: float = NAN


class OffboardClient:
    def __init__(self, node: Node):
        self._logger = node.get_logger().get_child("offboard")

        self._state = State()
        self._node = node

        self._navigate_async_aclient = ActionClient(
            self._node, NavigateAsync, "/fcu_bridge/navigate_async"
        )

        self._state_sub = self._node.create_subscription(
            State, "/fcu_bridge/state", self._state_callback, 10
        )

        self._arm_disarm_client = self._node.create_client(
            ArmDisarm, "/fcu_bridge/arm_disarm"
        )
        self._land_client = self._node.create_client(Land, "/fcu_bridge/land")
        self._set_position_client = self._node.create_client(
            SetPosition, "/fcu_bridge/set_position"
        )
        self._navigate_client = self._node.create_client(
            Navigate, "/fcu_bridge/navigate"
        )

        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, node)

    def is_armed(self) -> bool:
        return self._state.is_armed

    def flight_mode(self) -> str:
        return self._state.mode

    def arm_disarm(self, arm: bool) -> bool:
        req = ArmDisarm.Request()
        req.arm = arm

        return self.__wait_service_call(self._arm_disarm_client, req)

    def arm(self) -> bool:
        return self.arm_disarm(True)

    def disarm(self) -> bool:
        return self.arm_disarm(False)

    def land(self) -> bool:
        req = Land.Request()
        return self.__wait_service_call(self._land_client, req)

    def set_position(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
    ) -> bool:
        req = SetPosition.Request()
        req.header.frame_id = frame_id
        req.header.stamp = self._node.get_clock().now().to_msg()

        req.pose = self.__fill_pose(x, y, z, yaw)

        return self.__wait_service_call(self._set_position_client, req)

    def navigate(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
        speed: float = 0.5,
    ) -> bool:
        req = Navigate.Request()
        req.header.frame_id = frame_id
        req.header.stamp = self._node.get_clock().now().to_msg()

        req.speed = speed
        req.pose = self.__fill_pose(x, y, z, yaw)

        return self.__wait_service_call(self._navigate_client, req)

    def navigate_wait(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
        speed: float = 0.5,
    ) -> bool:
        goal = NavigateAsync.Goal()
        goal.header.frame_id = frame_id
        goal.header.stamp = self._node.get_clock().now().to_msg()

        goal.speed = speed
        goal.pose = self.__fill_pose(x, y, z, yaw)

        self._navigate_async_aclient.wait_for_server()

        goal_uuid = UUID(uuid=list(uuid_lib.uuid4().bytes))
        goal_uuid_string = str(uuid_lib.UUID(bytes=bytes(goal_uuid.uuid)))
        self._logger.debug(f"Navigate async goal UUID: {goal_uuid_string}")

        # The same UUID binds the bond to this action only.
        bond_id = f"navigate_async:{goal_uuid_string}"
        navigate_bond = bondpy.Bond(self._node, NAVIGATE_BOND_TOPIC, bond_id)
        navigate_bond.set_connect_timeout(NAVIGATE_BOND_CONNECT_TIMEOUT)
        navigate_bond.set_heartbeat_period(NAVIGATE_BOND_HEARTBEAT_PERIOD)
        navigate_bond.set_heartbeat_timeout(NAVIGATE_BOND_HEARTBEAT_TIMEOUT)
        navigate_bond.start()

        try:
            helper = ActionHelper(
                self._navigate_async_aclient, goal, goal_uuid)
            status = helper.wait()

            if status is ActionStatus.REJECTED:
                raise RuntimeError(helper.message)

            if status is not ActionStatus.SUCCEEDED:
                raise RuntimeError(
                    f"NavigateAsync {status.name}: {helper.message}")

            if not helper.result.success:
                raise RuntimeError(helper.result.message)

            return helper.result.success
        finally:
            navigate_bond.break_bond()

    def get_position(self, from_frame: str = "map") -> DronePosition:
        t = self._tf_buffer.lookup_transform(from_frame, "base_link", Time())

        rpy = euler_from_quaternion((
            t.transform.rotation.x,
            t.transform.rotation.y,
            t.transform.rotation.z,
            t.transform.rotation.w,
        ))

        return DronePosition(
            x=t.transform.translation.x,
            y=t.transform.translation.y,
            z=t.transform.translation.z,
            roll=rpy[0],
            pitch=rpy[1],
            yaw=rpy[2]
        )

    def __fill_pose(self, x, y, z, yaw) -> Pose:
        pose = Pose()
        pose.position.x = float(x)
        pose.position.y = float(y)
        pose.position.z = float(z)

        if yaw != NAN:
            q = quaternion_from_euler(0.0, 0.0, yaw)
            pose.orientation.x = q[0]
            pose.orientation.y = q[1]
            pose.orientation.z = q[2]
            pose.orientation.w = q[3]
        else:
            pose.orientation.w = NAN

        return pose

    def _state_callback(self, msg: State):
        self._state = msg

    def __wait_service_call(self, srv, request, timeout=1.0) -> bool:
        future = srv.call_async(request)
        result = wait_future(future, timeout=timeout)

        if not result:
            self._node.get_logger().error("Service not response")
            return False

        if not result.success:
            self._node.get_logger().error(
                f"`{srv.service_name}`: {result.message}")

        return result.success
