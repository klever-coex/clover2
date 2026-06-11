from dataclasses import dataclass, field
from enum import StrEnum


class FCUInterface(StrEnum):
    USB = "usb"
    UART = "uart"
    TCP = "tcp"
    UDP = "udp"


@dataclass
class FCUBridgeConfig:
    interface: FCUInterface = field(default=FCUInterface("usb"))
