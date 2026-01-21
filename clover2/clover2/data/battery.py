from dataclasses import dataclass


@dataclass
class Battery:
    voltage: float
    percentage: float
    current: float

    def __str__(self) -> str:
        return (
            f"Battery(voltage=({self.voltage:.2f}), "
            f"percentage=({self.percentage:.1f}%), "
            f"current=({self.current:.2f}))"
        )
