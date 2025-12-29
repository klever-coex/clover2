"""Coordinate transformation utilities - no ROS dependencies."""

import math
from typing import Optional

from .types import Pose, Point


class CoordinateManager:
    """Manages coordinate frame transformations."""

    def __init__(self):
        """Initialize coordinate manager with default pose."""
        self._current_pose: Optional[Pose] = None

    def set_current_pose(self, pose: Pose) -> None:
        """Update the current drone pose."""
        self._current_pose = pose

    def get_current_pose(self) -> Optional[Pose]:
        """Get the current drone pose."""
        return self._current_pose

    def transform(self, pose: Pose, to_frame: str) -> Pose:
        """
        Transform a pose to the target frame.

        Args:
            pose: The pose to transform
            to_frame: Target frame ("map", "body", or "maps")

        Returns:
            Transformed pose in the target frame

        Raises:
            ValueError: If current pose is not set and transformation requires it
            ValueError: If target frame is unknown
        """
        if pose.frame_id == to_frame:
            return pose

        if to_frame == "map":
            return self._to_map(pose)
        elif to_frame == "body":
            return self._to_body(pose)
        elif to_frame == "maps":
            return self._to_maps(pose)
        else:
            raise ValueError(f"Unknown target frame: {to_frame}")

    def _to_map(self, pose: Pose) -> Pose:
        """Transform pose to map frame."""
        if pose.frame_id == "map":
            return pose
        elif pose.frame_id == "body":
            return self._body_to_map(pose)
        elif pose.frame_id == "maps":
            # Maps is similar to body but aligned with map axes
            # For now, treat it as relative to current position
            if self._current_pose is None:
                raise ValueError("Current pose must be set for maps->map transformation")
            return Pose(
                x=self._current_pose.x + pose.x,
                y=self._current_pose.y + pose.y,
                z=self._current_pose.z + pose.z,
                yaw=self._current_pose.yaw + pose.yaw,
                frame_id="map",
            )
        else:
            raise ValueError(f"Cannot transform from {pose.frame_id} to map")

    def _to_body(self, pose: Pose) -> Pose:
        """Transform pose to body frame."""
        if pose.frame_id == "body":
            return pose
        elif pose.frame_id == "map":
            return self._map_to_body(pose)
        elif pose.frame_id == "maps":
            # Maps to body: rotate by current yaw
            if self._current_pose is None:
                raise ValueError("Current pose must be set for maps->body transformation")
            return self._map_to_body(
                Pose(
                    x=self._current_pose.x + pose.x,
                    y=self._current_pose.y + pose.y,
                    z=self._current_pose.z + pose.z,
                    yaw=self._current_pose.yaw + pose.yaw,
                    frame_id="map",
                )
            )
        else:
            raise ValueError(f"Cannot transform from {pose.frame_id} to body")

    def _to_maps(self, pose: Pose) -> Pose:
        """Transform pose to maps frame (relative to current position, map-aligned)."""
        if pose.frame_id == "maps":
            return pose
        elif pose.frame_id == "map":
            if self._current_pose is None:
                raise ValueError("Current pose must be set for map->maps transformation")
            return Pose(
                x=pose.x - self._current_pose.x,
                y=pose.y - self._current_pose.y,
                z=pose.z - self._current_pose.z,
                yaw=pose.yaw - self._current_pose.yaw,
                frame_id="maps",
            )
        elif pose.frame_id == "body":
            # Body to maps: first convert to map, then to maps
            map_pose = self._body_to_map(pose)
            return self._to_maps(map_pose)
        else:
            raise ValueError(f"Cannot transform from {pose.frame_id} to maps")

    def _map_to_body(self, pose: Pose) -> Pose:
        """Transform from map frame to body frame."""
        if self._current_pose is None:
            raise ValueError("Current pose must be set for map->body transformation")

        # Calculate relative position
        dx = pose.x - self._current_pose.x
        dy = pose.y - self._current_pose.y
        dz = pose.z - self._current_pose.z

        # Rotate by negative current yaw (body frame: x=forward, y=left)
        cos_yaw = math.cos(-self._current_pose.yaw)
        sin_yaw = math.sin(-self._current_pose.yaw)

        # Rotation matrix for 2D rotation
        body_x = dx * cos_yaw - dy * sin_yaw
        body_y = dx * sin_yaw + dy * cos_yaw

        # Relative yaw
        body_yaw = pose.yaw - self._current_pose.yaw

        return Pose(x=body_x, y=body_y, z=dz, yaw=body_yaw, frame_id="body")

    def _body_to_map(self, pose: Pose) -> Pose:
        """Transform from body frame to map frame."""
        if self._current_pose is None:
            raise ValueError("Current pose must be set for body->map transformation")

        # Rotate body coordinates by current yaw
        cos_yaw = math.cos(self._current_pose.yaw)
        sin_yaw = math.sin(self._current_pose.yaw)

        # Rotation matrix for 2D rotation
        map_x = pose.x * cos_yaw - pose.y * sin_yaw
        map_y = pose.x * sin_yaw + pose.y * cos_yaw

        # Absolute position
        abs_x = self._current_pose.x + map_x
        abs_y = self._current_pose.y + map_y
        abs_z = self._current_pose.z + pose.z

        # Absolute yaw
        abs_yaw = self._current_pose.yaw + pose.yaw

        return Pose(x=abs_x, y=abs_y, z=abs_z, yaw=abs_yaw, frame_id="map")




