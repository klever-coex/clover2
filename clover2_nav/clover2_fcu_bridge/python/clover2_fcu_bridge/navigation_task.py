import uuid as uuid_lib
from enum import Enum, auto
from typing import Any

from bondpy import bondpy
from rclpy.action import ActionClient
from rclpy.node import Node
from unique_identifier_msgs.msg import UUID

from ._action_helper import ActionHelper, ActionStatus

NAVIGATE_BOND_TOPIC = "/fcu_bridge/bond"
NAVIGATE_BOND_CONNECT_TIMEOUT = 2.0
NAVIGATE_BOND_HEARTBEAT_PERIOD = 0.2
NAVIGATE_BOND_HEARTBEAT_TIMEOUT = 1.0


class NavigationStatus(Enum):
    PENDING = auto()
    ACTIVE = auto()
    CANCELING = auto()
    REJECTED = auto()
    SUCCEEDED = auto()
    CANCELED = auto()
    ABORTED = auto()


class NavigationError(RuntimeError):
    pass


class NavigationRejectedError(NavigationError):
    pass


class NavigationCanceledError(NavigationError):
    pass


class NavigationAbortedError(NavigationError):
    pass


class NavigationTimeoutError(NavigationError):
    pass


_STATUS_MAP: dict[ActionStatus, NavigationStatus] = {
    ActionStatus.PENDING: NavigationStatus.PENDING,
    ActionStatus.ACTIVE: NavigationStatus.ACTIVE,
    ActionStatus.CANCELING: NavigationStatus.CANCELING,
    ActionStatus.REJECTED: NavigationStatus.REJECTED,
    ActionStatus.SUCCEEDED: NavigationStatus.SUCCEEDED,
    ActionStatus.CANCELED: NavigationStatus.CANCELED,
    ActionStatus.ABORTED: NavigationStatus.ABORTED,
}


class NavigationTask:
    def __init__(self, node: Node, action: ActionClient, goal: Any):
        self._logger = node.get_logger().get_child("navigation_task")
        self._bond_closed = False

        goal_uuid = UUID(uuid=list(uuid_lib.uuid4().bytes))
        goal_uuid_string = str(uuid_lib.UUID(bytes=bytes(goal_uuid.uuid)))
        self._logger.debug(f"Navigate async goal UUID: {goal_uuid_string}")

        bond_id = f"navigate_async:{goal_uuid_string}"
        self._bond = bondpy.Bond(node, NAVIGATE_BOND_TOPIC, bond_id)
        self._bond.set_connect_timeout(NAVIGATE_BOND_CONNECT_TIMEOUT)
        self._bond.set_heartbeat_period(NAVIGATE_BOND_HEARTBEAT_PERIOD)
        self._bond.set_heartbeat_timeout(NAVIGATE_BOND_HEARTBEAT_TIMEOUT)
        self._bond.start()

        self._helper = ActionHelper(action, goal, goal_uuid)
        self._helper.add_done_callback(lambda _: self._close_bond())

    @property
    def status(self) -> NavigationStatus:
        return _STATUS_MAP[self._helper.status]

    @property
    def result(self) -> Any:
        return self._helper.result

    @property
    def message(self) -> str:
        return self._helper.message

    def cancel(self) -> bool:
        return self._helper.cancel()

    def wait(self, timeout: float | None = None) -> bool:
        status = self._helper.wait(timeout)
        if status is ActionStatus.TIMEOUT:
            raise NavigationTimeoutError(
                "Navigation task did not finish before the timeout"
            )
        if status is ActionStatus.SUCCEEDED:
            if self.result is not None and not self.result.success:
                raise NavigationAbortedError(self.message)
            return True
        if status is ActionStatus.REJECTED:
            raise NavigationRejectedError(self.message)
        if status is ActionStatus.CANCELED:
            raise NavigationCanceledError(self.message)
        raise NavigationAbortedError(self.message)

    def _close_bond(self) -> None:
        if self._bond_closed:
            return

        self._bond_closed = True
        self._bond.break_bond()