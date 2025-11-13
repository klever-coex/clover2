from enum import Enum


class FlightMode(Enum):
    MANUAL = "manual"
    STABILIZE = "stabilize"
    ALT_HOLD = "alt_hold"
    POSITION = "position"
    OFFBOARD = "offboard"
    RETURN_TO_LAUNCH = "return_to_launch"
    LAND = "land"
    HOLD = "hold"
    UNKNOWN = "unknown"
