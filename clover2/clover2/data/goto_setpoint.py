from dataclasses import dataclass
from numbers import Real
from typing import Union

SetpointValue = Union[Real, None]


@dataclass
class GoToSetpoint:
    x: SetpointValue
    y: SetpointValue
    z: SetpointValue
    yaw: SetpointValue
    speed: SetpointValue
