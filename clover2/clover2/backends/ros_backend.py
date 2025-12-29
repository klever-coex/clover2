"""ROS backend implementation with lazy imports."""

import sys
import threading
import time
from typing import Optional, Any

from ..core.interfaces import DroneBackend
from ..core.types import Pose


class RosBackend(DroneBackend):
    """ROS2 MAVROS-based backend for drone control."""

    def __init__(self, node: Optional[Any] = None, node_name: str = "clover2_backend"):
        """
        Initialize ROS backend.

        Args:
            node: Optional ROS2 node instance. If None, creates a new node.
            node_name: Name for the ROS node if creating a new one
        """
        self._node = node
        self._node_name = node_name
        self._owns_node = node is None
        self._running = False
        self._ros_thread: Optional[threading.Thread] = None
        self._current_pose: Optional[Pose] = None
        self._lock = threading.Lock()

        # Lazy imports - only import when needed
        self._rclpy: Optional[Any] = None
        self._geometry_msgs: Optional[Any] = None
        self._mavros_msgs: Optional[Any] = None

    def _lazy_import_ros(self) -> None:
        """Lazy import ROS modules."""
        if self._rclpy is None:
            try:
                import rclpy
                from rclpy.node import Node
                import geometry_msgs.msg
                from geometry_msgs.msg import Point, Quaternion
                from mavros_msgs.msg import PositionTarget
                from mavros_msgs.srv import SetMode, CommandBool
                from std_msgs.msg import Header

                self._rclpy = rclpy
                self._Node = Node
                self._geometry_msgs = geometry_msgs.msg
                self._Point = Point
                self._Quaternion = Quaternion
                self._PositionTarget = PositionTarget
                self._SetMode = SetMode
                self._CommandBool = CommandBool
                self._Header = Header
            except ImportError as e:
                raise ImportError(
                    "ROS2 dependencies not available. "
                    "Install rclpy, geometry_msgs, and mavros_msgs. "
                    f"Original error: {e}"
                )

    def start(self) -> None:
        """Start the ROS backend."""
        self._lazy_import_ros()

        # Initialize ROS if we own the node
        if self._owns_node:
            if not self._rclpy.ok():
                self._rclpy.init()
            self._node = self._Node(self._node_name)

        if self._node is None:
            raise RuntimeError("ROS node not available")

        # Create publishers and subscribers
        self._pose_pub = self._node.create_publisher(
            self._PositionTarget, "/mavros/setpoint_position/local", 10
        )
        self._pose_sub = self._node.create_subscription(
            self._geometry_msgs.PoseStamped,
            "/mavros/local_position/pose",
            self._pose_callback,
            10,
        )

        # Create service clients
        self._arming_client = self._node.create_client(
            self._CommandBool, "/mavros/cmd/arming"
        )
        self._set_mode_client = self._node.create_client(
            self._SetMode, "/mavros/set_mode"
        )

        self._running = True

        # Start ROS spin thread if we own the node
        if self._owns_node:
            self._ros_thread = threading.Thread(target=self._ros_spin, daemon=True)
            self._ros_thread.start()

    def _ros_spin(self) -> None:
        """Background thread for ROS spinning."""
        while self._running and self._rclpy.ok():
            self._rclpy.spin_once(self._node, timeout_sec=0.1)

    def _pose_callback(self, msg: Any) -> None:
        """Callback for pose updates."""
        import math

        # Extract quaternion and convert to yaw
        q = msg.pose.orientation
        yaw = math.atan2(
            2.0 * (q.w * q.z + q.x * q.y),
            1.0 - 2.0 * (q.y * q.y + q.z * q.z),
        )

        with self._lock:
            self._current_pose = Pose(
                x=msg.pose.position.x,
                y=msg.pose.position.y,
                z=msg.pose.position.z,
                yaw=yaw,
                frame_id="map",
            )

    def arm(self, value: bool) -> bool:
        """Arm or disarm the drone."""
        if not self._running or not self._node:
            return False

        self._lazy_import_ros()

        request = self._CommandBool.Request()
        request.value = value

        future = self._arming_client.call_async(request)
        timeout = 5.0
        start_time = time.time()

        while not future.done() and (time.time() - start_time) < timeout:
            if self._owns_node:
                self._rclpy.spin_once(self._node, timeout_sec=0.1)
            else:
                time.sleep(0.1)

        if future.done():
            try:
                return future.result().success
            except Exception:
                return False
        return False

    def land(self) -> bool:
        """Command the drone to land."""
        if not self._running or not self._node:
            return False

        self._lazy_import_ros()

        request = self._SetMode.Request()
        request.custom_mode = "AUTO.LAND"

        future = self._set_mode_client.call_async(request)
        timeout = 5.0
        start_time = time.time()

        while not future.done() and (time.time() - start_time) < timeout:
            if self._owns_node:
                self._rclpy.spin_once(self._node, timeout_sec=0.1)
            else:
                time.sleep(0.1)

        if future.done():
            try:
                return future.result().mode_sent
            except Exception:
                return False
        return False

    def set_target_pose(self, pose: Pose) -> bool:
        """Set the target pose for the drone."""
        if not self._running or not self._node:
            return False

        self._lazy_import_ros()

        msg = self._PositionTarget()
        msg.header = self._Header()
        msg.header.stamp = self._node.get_clock().now().to_msg()
        msg.header.frame_id = "map"
        msg.type_mask = (
            self._PositionTarget.IGNORE_VX
            | self._PositionTarget.IGNORE_VY
            | self._PositionTarget.IGNORE_VZ
            | self._PositionTarget.IGNORE_AFX
            | self._PositionTarget.IGNORE_AFY
            | self._PositionTarget.IGNORE_AFZ
            | self._PositionTarget.IGNORE_YAW_RATE
        )
        msg.position = self._Point(x=float(pose.x), y=float(pose.y), z=float(pose.z))
        msg.yaw = float(pose.yaw)

        self._pose_pub.publish(msg)
        return True

    def get_telemetry(self) -> Optional[Pose]:
        """Get current drone telemetry."""
        with self._lock:
            if self._current_pose:
                return Pose(
                    x=self._current_pose.x,
                    y=self._current_pose.y,
                    z=self._current_pose.z,
                    yaw=self._current_pose.yaw,
                    frame_id=self._current_pose.frame_id,
                )
            return None

    def node(self) -> Optional[Any]:
        """Get the ROS node object."""
        return self._node

    def stop(self) -> None:
        """Stop the backend."""
        self._running = False
        if self._ros_thread and self._ros_thread.is_alive():
            self._ros_thread.join(timeout=1.0)

        if self._owns_node and self._node:
            self._node.destroy_node()
            if self._rclpy and self._rclpy.ok():
                self._rclpy.shutdown()

