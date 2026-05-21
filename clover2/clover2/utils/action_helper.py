import threading
from enum import Enum, auto
from typing import Any

from action_msgs.msg import GoalStatus
from rclpy.action import ActionClient
from rclpy.task import Future


class ActionStatus(Enum):
    REJECTED = auto()
    SUCCEEDED = auto()
    ABORTED = auto()
    CANCELED = auto()
    TIMEOUT = auto()


_GOAL_STATUS_MAP: dict[int, ActionStatus] = {
    GoalStatus.STATUS_SUCCEEDED: ActionStatus.SUCCEEDED,
    GoalStatus.STATUS_ABORTED: ActionStatus.ABORTED,
    GoalStatus.STATUS_CANCELED: ActionStatus.CANCELED,
}


class ActionHelper:
    def __init__(self, action: ActionClient, goal: Any) -> None:
        self._event = threading.Event()
        self._status: ActionStatus = ActionStatus.REJECTED
        self._result: Any = None
        self._message: str = ""

        goal_future: Future = action.send_goal_async(goal)
        goal_future.add_done_callback(self._on_goal_response)

    def wait(self, timeout: float | None = None) -> ActionStatus:
        if self._event.wait(timeout=timeout):
            return self._status
        return ActionStatus.TIMEOUT

    @property
    def status(self) -> ActionStatus:
        return self._status

    @property
    def result(self) -> Any:
        return self._result

    @property
    def message(self) -> str:
        return self._message

    def ok(self) -> bool:
        return self._status is ActionStatus.SUCCEEDED

    def _on_goal_response(self, future: Future) -> None:
        goal_handle = future.result()
        if not goal_handle.accepted:
            self._message = goal_handle.reject_reason() or "Goal rejected"
            self._event.set()
            return

        get_result = goal_handle.get_result_async()
        get_result.add_done_callback(self._on_result)

    def _on_result(self, future: Future) -> None:
        response = future.result()
        self._status = _GOAL_STATUS_MAP.get(response.status, ActionStatus.ABORTED)
        self._result = response.result
        self._event.set()
