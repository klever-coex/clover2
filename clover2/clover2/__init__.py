"""Clover2 - High-level Python library for drone control."""

import sys
import time
from typing import Optional, Any, Union
from numbers import Real

from .core.types import Pose, Point
from .core.coordinates import CoordinateManager
from .core.interfaces import DroneBackend
from .backends.tcp_backend import TcpBackend

# Try to import ROS backend lazily
try:
    from .backends.ros_backend import RosBackend
except ImportError:
    RosBackend = None

__title__ = "clover2"
__all__ = ["Clover2"]


class Clover2:
    """
    High-level facade for drone control.

    This class provides a unified interface for controlling drones through
    various backends (ROS2/MAVROS or TCP). The library works both with and
    without ROS2 dependencies.

    Examples:
        Basic usage with auto-detection:
            >>> clover = Clover2()
            >>> clover.arm(True)
            >>> clover.move(x=1.0, y=0.0, z=1.0)
            >>> clover.land()

        Usage with explicit backend:
            >>> clover = Clover2(backend="tcp", host="192.168.1.100", port=5760)
            >>> clover.move(x=2.0, wait=True)

        Usage with ROS node:
            >>> import rclpy
            >>> from rclpy.node import Node
            >>> node = Node("my_node")
            >>> clover = Clover2(node=node)
    """

    def __init__(
        self,
        backend: str = "auto",
        node: Optional[Any] = None,
        **kwargs,
    ):
        """
        Initialize Clover2 facade.

        Args:
            backend: Backend type ("auto", "ros", or "tcp"). Defaults to "auto".
            node: Optional ROS2 node instance. If provided, ROS backend will be used.
            **kwargs: Additional backend-specific arguments:
                - For TCP backend: host, port, timeout
                - For ROS backend: node_name

        Raises:
            ValueError: If backend type is invalid
            ImportError: If ROS backend is requested but ROS dependencies are unavailable
            ConnectionError: If backend cannot connect
        """
        self._coordinate_manager = CoordinateManager()
        self._backend: Optional[DroneBackend] = None

        # Auto-detect backend
        if backend == "auto":
            backend = self._auto_detect_backend(node)

        # Initialize backend
        if backend == "ros":
            if RosBackend is None:
                raise ImportError(
                    "ROS backend requested but ROS2 dependencies are not available. "
                    "Install rclpy, geometry_msgs, and mavros_msgs."
                )
            node_name = kwargs.get("node_name", "clover2")
            self._backend = RosBackend(node=node, node_name=node_name)
        elif backend == "tcp":
            host = kwargs.get("host", "127.0.0.1")
            port = kwargs.get("port", 5760)
            timeout = kwargs.get("timeout", 1.0)
            self._backend = TcpBackend(host=host, port=port, timeout=timeout)
        else:
            raise ValueError(f"Unknown backend type: {backend}")

        # Start backend
        try:
            self._backend.start()
        except Exception as e:
            raise ConnectionError(f"Failed to start backend: {e}")

        # Update coordinate manager with initial pose
        self._update_coordinate_manager()

    def _auto_detect_backend(self, node: Optional[Any]) -> str:
        """
        Auto-detect which backend to use.

        Returns:
            "ros" if ROS is available and node is provided or rclpy is in sys.modules,
            "tcp" otherwise
        """
        # If node is provided, use ROS
        if node is not None:
            return "ros"

        # Check if rclpy is already imported
        if "rclpy" in sys.modules:
            return "ros"

        # Try to import rclpy
        try:
            import rclpy  # noqa: F401
            return "ros"
        except ImportError:
            return "tcp"

    def _update_coordinate_manager(self) -> None:
        """Update coordinate manager with current pose from backend."""
        pose = self._backend.get_telemetry()
        if pose:
            self._coordinate_manager.set_current_pose(pose)

    def move(
        self,
        x: Optional[Union[Real, float]] = None,
        y: Optional[Union[Real, float]] = None,
        z: Optional[Union[Real, float]] = None,
        yaw: Optional[Union[Real, float]] = None,
        wait: bool = False,
        frame_id: str = "map",
    ) -> bool:
        """
        Move the drone to a target position.

        Args:
            x: Target x coordinate (None to keep current)
            y: Target y coordinate (None to keep current)
            z: Target z coordinate (None to keep current)
            yaw: Target yaw angle in radians (None to keep current)
            wait: If True, block until target is reached (within tolerance)
            frame_id: Coordinate frame ("map", "body", or "maps")

        Returns:
            True if command was sent successfully, False otherwise
        """
        # Get current pose
        current_pose = self._backend.get_telemetry()
        if current_pose is None:
            raise RuntimeError("Cannot get current pose from backend")

        # Update coordinate manager
        self._coordinate_manager.set_current_pose(current_pose)

        # Calculate target pose
        target_pose = Pose(
            x=x if x is not None else current_pose.x,
            y=y if y is not None else current_pose.y,
            z=z if z is not None else current_pose.z,
            yaw=yaw if yaw is not None else current_pose.yaw,
            frame_id=frame_id,
        )

        # Transform to map frame if needed
        if frame_id != "map":
            target_pose = self._coordinate_manager.transform(target_pose, "map")

        # Send command
        success = self._backend.set_target_pose(target_pose)

        # Wait for target if requested
        if wait and success:
            self._wait_for_target(target_pose)

        return success

    def _wait_for_target(self, target_pose: Pose, tolerance: float = 0.2, timeout: float = 30.0) -> None:
        """
        Wait until drone reaches target pose.

        Args:
            target_pose: Target pose to wait for
            tolerance: Distance tolerance in meters
            timeout: Maximum time to wait in seconds
        """
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            current_pose = self._backend.get_telemetry()
            if current_pose:
                distance = current_pose.distance_to(target_pose)
                if distance < tolerance:
                    return
            time.sleep(0.1)

        # Timeout reached
        raise TimeoutError(f"Timeout waiting for target pose (tolerance: {tolerance}m)")

    def arm(self, value: bool = True) -> bool:
        """
        Arm or disarm the drone.

        Args:
            value: True to arm, False to disarm. Defaults to True.

        Returns:
            True if successful, False otherwise
        """
        return self._backend.arm(value)

    def disarm(self) -> bool:
        """
        Disarm the drone.

        Returns:
            True if successful, False otherwise
        """
        return self.arm(False)

    def land(self) -> bool:
        """
        Command the drone to land.

        Returns:
            True if command was sent successfully, False otherwise
        """
        return self._backend.land()

    def get_pose(self, frame_id: str = "map") -> Optional[Pose]:
        """
        Get current drone pose.

        Args:
            frame_id: Desired coordinate frame ("map", "body", or "maps")

        Returns:
            Current pose in the requested frame, or None if not available
        """
        pose = self._backend.get_telemetry()
        if pose is None:
            return None

        # Transform to requested frame if needed
        if frame_id != pose.frame_id:
            try:
                pose = self._coordinate_manager.transform(pose, frame_id)
            except ValueError:
                # If transformation fails, return None
                return None

        return pose

    def get_orientation(self) -> Optional[float]:
        """
        Get current yaw orientation.

        Returns:
            Current yaw angle in radians, or None if not available
        """
        pose = self.get_pose()
        return pose.yaw if pose else None

    def node(self) -> Optional[Any]:
        """
        Get the ROS node object if available.

        Returns:
            ROS node object or None if not using ROS backend
        """
        return self._backend.node()

    def stop(self) -> None:
        """Stop the backend and cleanup resources."""
        if hasattr(self._backend, "stop"):
            self._backend.stop()
