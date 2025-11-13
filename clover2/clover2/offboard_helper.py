from enum import Enum

import rclpy
from rclpy.node import Node

from clover2.client import ClientBase
from clover2.data import GoToSetpoint


class OffboardMode(Enum):
    NONE = "none"
    SPEED = "speed"
    POSITION = "position"


class OffboardHelper:
    def __init__(self, node: Node, client: ClientBase):
        self._logger = node.get_logger().get_child("offboard")

        self._client = client
        self._current_mode = OffboardMode.POSITION

        self._position_setopint = GoToSetpoint(None, None, None, None, None)

        self._offboard_timer = node.create_timer(
            1.0 / 50.0, self._offboard_timer_callback
        )

        self._logger.info("Offboard helper started")

    def _offboard_timer_callback(self):
        self._update_offboard()

    def _update_offboard(self):
        match (self._current_mode):
            case OffboardMode.POSITION:
                self._send_position()
            case OffboardMode.SPEED:
                self._send_speed()
            case _:
                pass

    def _send_position(self):
        self._client.send_state_impl(self._position_setopint, "")

    def _send_speed(self):
        pass

    def move(self, setpoint: GoToSetpoint):
        state = self._client.get_state()

        if setpoint.x is None:
            setpoint.x = state.x()

        if setpoint.y is None:
            setpoint.y = state.y()

        if setpoint.z is None:
            setpoint.z = state.z()

        self._position_setopint = setpoint
