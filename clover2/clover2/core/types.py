"""Core types for clover2 library - no ROS dependencies."""

from dataclasses import dataclass
from typing import Optional


@dataclass
class Point:
    """3D point representation."""

    x: float
    y: float
    z: float

    def __iter__(self):
        """Allow unpacking: x, y, z = point."""
        return iter((self.x, self.y, self.z))

    def __str__(self) -> str:
        return f"Point(x={self.x:.2f}, y={self.y:.2f}, z={self.z:.2f})"


@dataclass
class Pose:
    """Pose representation with position and yaw orientation."""

    x: float
    y: float
    z: float
    yaw: float
    frame_id: str = "map"

    def __post_init__(self):
        """Normalize yaw to [-pi, pi] range."""
        import math
        self.yaw = math.atan2(math.sin(self.yaw), math.cos(self.yaw))

    @property
    def position(self) -> Point:
        """Get position as Point."""
        return Point(x=self.x, y=self.y, z=self.z)

    def distance_to(self, other: "Pose") -> float:
        """Calculate Euclidean distance to another pose."""
        import math
        dx = self.x - other.x
        dy = self.y - other.y
        dz = self.z - other.z
        return math.sqrt(dx * dx + dy * dy + dz * dz)

    def __str__(self) -> str:
        import math
        yaw_deg = math.degrees(self.yaw)
        return (
            f"Pose(x={self.x:.2f}, y={self.y:.2f}, z={self.z:.2f}, "
            f"yaw={yaw_deg:.1f}°, frame={self.frame_id})"
        )




