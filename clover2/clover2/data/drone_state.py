from dataclasses import dataclass
from typing import Tuple
import math


@dataclass
class DroneState:
    position: Tuple[float, float, float]
    orientation: Tuple[float, float, float]
    velocity: Tuple[float, float, float]

    def x(self) -> float:
        return self.position[0]

    def y(self) -> float:
        return self.position[1]

    def z(self) -> float:
        return self.position[2]

    def get_position(self) -> Tuple[float, float, float]:
        return self.position

    def roll(self) -> float:
        return self.orientation[0]

    def pitch(self) -> float:
        return self.orientation[1]

    def yaw(self) -> float:
        return self.orientation[2]

    def roll_d(self) -> float:
        return math.degrees(self.roll())

    def pitch_d(self) -> float:
        return math.degrees(self.pitch())

    def yaw_d(self) -> float:
        return math.degrees(self.yaw())

    def get_orientation(self) -> Tuple[float, float, float]:
        return self.orientation

    def get_orientation_degrees(self) -> Tuple[float, float, float]:
        return (self.roll_d(), self.pitch_d(), self.yaw_d())

    def get_yaw_degrees(self) -> float:
        return math.degrees(self.yaw())

    def vx(self) -> float:
        return self.velocity[0]

    def vy(self) -> float:
        return self.velocity[1]

    def vz(self) -> float:
        return self.velocity[2]

    def get_velocity(self) -> Tuple[float, float, float]:
        return self.velocity

    def to_dict(self) -> dict:
        return {
            "position": {"x": self.x(), "y": self.y(), "z": self.z()},
            "orientation": {
                "roll_rad": self.roll(),
                "pitch_rad": self.pitch(),
                "yaw_rad": self.yaw(),
            },
            "velocity": {
                "vx": self.vx(),
                "vy": self.vy(),
                "vz": self.vz(),
            },
        }

    def __str__(self) -> str:
        return (
            f"DroneState(pos=({self.x():.2f}, {self.y():.2f}, {self.z():.2f}), "
            f"orient_deg=({self.roll_d():.1f}, {self.pitch_d():.1f}, {self.yaw_d():.1f}), "
            f"vel=({self.vx():.2f}, {self.vy():.2f}, {self.vz():.2f}))"
        )

    def copy(self) -> "DroneState":
        return DroneState(
            position=self.position, orientation=self.orientation, velocity=self.velocity
        )
