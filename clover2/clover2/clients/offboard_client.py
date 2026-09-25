import math

from clover2_nav_msgs.action import NavigateAsync
from clover2_nav_msgs.srv import ArmDisarm, Land, SetPosition
from geometry_msgs.msg import Pose
from rclpy.action import ActionClient
from rclpy.node import Node
from tf_transformations import quaternion_from_euler

from ..utils import wait_future
from .navigation_task import NavigationAbortedError, NavigationTask

NAN = float("nan")
NAVIGATE_ACTION_SERVER_TIMEOUT = 3.0


class OffboardClient:
    def __init__(self, node: Node):
        self._logger = node.get_logger().get_child("offboard")
        self._node = node

        self._navigate_async_action_client = ActionClient(
            self._node, NavigateAsync, "/fcu_bridge/navigate_async"
        )

        self._set_position_client = self._node.create_client(
            SetPosition, "/fcu_bridge/set_position"
        )
        self._arm_disarm_client = self._node.create_client(
            ArmDisarm, "/fcu_bridge/arm_disarm"
        )
        self._land_client = self._node.create_client(Land, "/fcu_bridge/land")

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
    ) -> NavigationTask:
        goal = NavigateAsync.Goal()
        goal.header.frame_id = frame_id
        goal.header.stamp = self._node.get_clock().now().to_msg()
        goal.speed = speed
        goal.pose = self.__fill_pose(x, y, z, yaw)

        if not self._navigate_async_action_client.wait_for_server(
            timeout_sec=NAVIGATE_ACTION_SERVER_TIMEOUT
        ):
            raise NavigationAbortedError(
                "NavigateAsync action server /fcu_bridge/navigate_async is unavailable"
            )

        return NavigationTask(self._node, self._navigate_async_action_client, goal)

    def navigate_wait(
        self,
        frame_id: str = "map",
        x: float = NAN,
        y: float = NAN,
        z: float = NAN,
        yaw: float = NAN,
        speed: float = 0.5,
    ) -> bool:
        return self.navigate(frame_id, x, y, z, yaw, speed).wait()

    def __fill_pose(self, x, y, z, yaw) -> Pose:
        pose = Pose()
        pose.position.x = float(x)
        pose.position.y = float(y)
        pose.position.z = float(z)

        if not math.isnan(yaw):
            q = quaternion_from_euler(0.0, 0.0, yaw)
            pose.orientation.x = q[0]
            pose.orientation.y = q[1]
            pose.orientation.z = q[2]
            pose.orientation.w = q[3]
        else:
            pose.orientation.w = NAN

        return pose

    def __wait_service_call(self, srv, request, timeout=1.0) -> bool:
        future = srv.call_async(request)
        result = wait_future(future, timeout=timeout)

        if not result:
            self._logger.error("Service did not respond")
            return False

        if not result.success:
            self._logger.error(f"`{srv.service_name}`: {result.message}")

        return result.success
