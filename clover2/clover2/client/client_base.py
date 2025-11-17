from abc import ABC, abstractmethod
from numbers import Real
from typing import Union
from enum import Enum
from rclpy.task import Future

from clover2.data import *

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
    def takeoff(self, z: Real, timeout: Real) -> bool:
        pass

    @abstractmethod
    def land(self, timeout: Real) -> bool:
        pass

    @abstractmethod
    def send_state_impl(self, setpoint: GoToSetpoint, frame_id: str):
        pass
