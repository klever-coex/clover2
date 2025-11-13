from dataclasses import dataclass


@dataclass
class Battery:
    voltage: float
    percentage: float
    current: float
