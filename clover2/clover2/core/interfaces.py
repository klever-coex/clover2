"""Abstract base class for drone backends."""

from abc import ABC, abstractmethod
from typing import Optional, Any

from .types import Pose


class DroneBackend(ABC):
    """Abstract base class for drone control backends."""

    @abstractmethod
    def start(self) -> None:
        """
        Start background loops and initialize the backend.

        This method should be called before using other methods.
        """
        pass

    @abstractmethod
    def arm(self, value: bool) -> bool:
        """
        Arm or disarm the drone.

        Args:
            value: True to arm, False to disarm

        Returns:
            True if successful, False otherwise
        """
        pass

    @abstractmethod
    def land(self) -> bool:
        """
        Command the drone to land.

        Returns:
            True if command was sent successfully, False otherwise
        """
        pass

    @abstractmethod
    def set_target_pose(self, pose: Pose) -> bool:
        """
        Set the target pose for the drone.

        Args:
            pose: Target pose in any supported frame

        Returns:
            True if command was sent successfully, False otherwise
        """
        pass

    @abstractmethod
    def get_telemetry(self) -> Optional[Pose]:
        """
        Get current drone telemetry.

        Returns:
            Current pose of the drone, or None if not available
        """
        pass

    def node(self) -> Optional[Any]:
        """
        Get the ROS node object if available.

        Returns:
            ROS node object or None if not using ROS backend
        """
        return None




