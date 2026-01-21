from abc import ABC, abstractmethod
from numbers import Real
from typing import Union
from enum import Enum
from rclpy.task import Future

from clover2.data import *
from geometry_msgs.msg import Point, Vector3

SetpointType = Union[Real, None]


class ClientType(Enum):
    MAVROS = "mavros"


class ClientBase(ABC):
    @abstractmethod
    def is_armed(self) -> bool:
        pass

    @abstractmethod
    def get_mode(self) -> FlightMode:
        pass

    @abstractmethod
    def set_mode(self, mode: FlightMode) -> bool:
        pass

    @abstractmethod
    def get_state(self) -> DroneState:
        pass

    @abstractmethod
    def get_battery(self) -> Battery:
        pass

    @abstractmethod
    def turn_motors_on_impl(self) -> Future:
        pass

    @abstractmethod
    def turn_motors_off_impl(self) -> Future:
        pass

    # Flight
    @abstractmethod
    def land(self, timeout: Real) -> bool:
        pass

    @abstractmethod
    def send_state_impl(self, setpoint: GoToSetpoint, frame_id: str):
        pass

    @abstractmethod
    def send_raw_local_impl(
        self, pos: Point, vel: Vector3, acc: Vector3, yaw: float, yaw_rate: float
    ):
        pass

    @abstractmethod
    def send_raw_body_impl(
        self, vel: Vector3, acc: Vector3, yaw: float, yaw_rate: float
    ):
        pass
