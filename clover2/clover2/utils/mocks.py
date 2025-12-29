"""Mock implementations for testing."""

import time
from typing import Optional

from ..core.interfaces import DroneBackend
from ..core.types import Pose


class MockPose:
    """Mock pose for testing."""

    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, yaw: float = 0.0):
        """Initialize mock pose."""
        self.x = x
        self.y = y
        self.z = z
        self.yaw = yaw


class MockBackend(DroneBackend):
    """Mock backend for testing - simulates drone behavior."""

    def __init__(self):
        """Initialize mock backend."""
        self._current_pose = Pose(x=0.0, y=0.0, z=0.0, yaw=0.0, frame_id="map")
        self._armed = False
        self._running = False
        self._target_pose: Optional[Pose] = None

    def start(self) -> None:
        """Start the mock backend."""
        self._running = True

    def arm(self, value: bool) -> bool:
        """Arm or disarm the mock drone."""
        self._armed = value
        return True

    def land(self) -> bool:
        """Command the mock drone to land."""
        if not self._armed:
            return False
        # Simulate landing by moving to z=0
        self._current_pose.z = 0.0
        return True

    def set_target_pose(self, pose: Pose) -> bool:
        """Set the target pose for the mock drone."""
        if not self._running:
            return False
        self._target_pose = pose
        # Simulate movement (simplified - instant movement)
        self._current_pose = Pose(
            x=pose.x,
            y=pose.y,
            z=pose.z,
            yaw=pose.yaw,
            frame_id=pose.frame_id,
        )
        return True

    def get_telemetry(self) -> Optional[Pose]:
        """Get current mock drone telemetry."""
        if not self._running:
            return None
        return Pose(
            x=self._current_pose.x,
            y=self._current_pose.y,
            z=self._current_pose.z,
            yaw=self._current_pose.yaw,
            frame_id=self._current_pose.frame_id,
        )

    def is_armed(self) -> bool:
        """Check if mock drone is armed."""
        return self._armed

    def get_target_pose(self) -> Optional[Pose]:
        """Get the current target pose."""
        return self._target_pose




