import threading
from types import NoneType
from typing import Any

from clover2_nav_msgs.action import NavigateAsync
from clover2_nav_msgs.msg import State
from clover2_nav_msgs.srv import ArmDisarm, Land, Navigate, SetPosition
from geometry_msgs.msg import Pose
from rclpy.action import ActionClient
from rclpy.node import Node
from rclpy.task import Future
from tf_transformations import quaternion_from_euler

from . import utils

NAN = float("nan")


class ActionHelper:
    def __init__(self, action: ActionClient, goal):
        self.event: threading.Event = threading.Event()
        self.result: NoneType = None
        self.message: str = "ok"
        self.goal_future: Future = action.send_goal_async(goal)
        self.goal_future.add_done_callback(self._response_callback)
        self.get_result_future: Future | NoneType = None

    def wait(self):
        self.event.wait()

    def _response_callback(self, future):
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.result = None
            self.message = "Navigate goal reject"
            self.event.set()
            return

        self.get_result_future = goal_handle.get_result_async()
        self.get_result_future.add_done_callback(self._result_callback)

    def _result_callback(self, future):
        self.result = future.result().result
        self.event.set()


class Offboard:
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

    def is_armed(self) -> bool:
        return self._state.is_armed

    def flight_mode(self) -> str:
        return self._state.mode

    def arm_disarm(self, arm: bool) -> bool:
        req = ArmDisarm.Request()
        req.arm = arm

        return self._wait_service_call(self._arm_disarm_client, req)

    def arm(self) -> bool:
        return self.arm_disarm(True)

    def disarm(self) -> bool:
        return self.arm_disarm(False)

    def land(self) -> bool:
        req = Land.Request()
        return self._wait_service_call(self._land_client, req)

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

        req.pose = self._fill_pose(x, y, z, yaw)

        return self._wait_service_call(self._set_position_client, req)

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
        req.pose = self._fill_pose(x, y, z, yaw)

        return self._wait_service_call(self._navigate_client, req)

    def navigate_async(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
        speed: float = 0.5,
    ):
        goal = NavigateAsync.Goal()
        goal.header.frame_id = frame_id
        goal.header.stamp = self._node.get_clock().now().to_msg()

        goal.speed = speed
        goal.pose = self._fill_pose(x, y, z, yaw)

        self._navigate_async_aclient.wait_for_server()

        helper = ActionHelper(self._navigate_async_aclient, goal)
        helper.wait()

        if helper.result is None:
            self._node.get_logger().error(helper.message)
            raise Exception(helper.message)

        if not helper.result.success:
            self._node.get_logger().error(helper.result.message)
            raise Exception(helper.result.message)

        return helper.result.success

    def _fill_pose(self, x, y, z, yaw) -> Pose:
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

    def _wait_service_call(self, srv, request, timeout=1.0) -> bool:
        future = srv.call_async(request)
        result = utils.wait_future(future, timeout=timeout)

        if not result:
            self._node.get_logger().error("Service not response")
            return False

        if not result.success:
            self._node.get_logger().error(f"`{srv.service_name}`: {result.message}")

        return result.success
