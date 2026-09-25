import threading
from enum import Enum, auto
from typing import Any

from action_msgs.msg import GoalStatus
from rclpy.action import ActionClient
from rclpy.task import Future
from unique_identifier_msgs.msg import UUID


class ActionStatus(Enum):
    PENDING = auto()
    ACTIVE = auto()
    CANCELING = auto()
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
    def __init__(
        self, action: ActionClient, goal: Any, goal_uuid: UUID | None = None
    ) -> None:
        self._event = threading.Event()
        self._lock = threading.Lock()
        self._status: ActionStatus = ActionStatus.PENDING
        self._result: Any = None
        self._message: str = ""
        self._goal_handle: Any = None
        self._cancel_requested = False
        self._cancel_sent = False
        self._done_callbacks: list = []

        try:
            goal_future: Future = action.send_goal_async(goal, goal_uuid=goal_uuid)
        except Exception as error:
            self._complete(ActionStatus.ABORTED, message=str(error))
            return

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

    def cancel(self) -> bool:
        with self._lock:
            if self._event.is_set():
                return False

            self._cancel_requested = True
            goal_handle = self._goal_handle
            if goal_handle is not None:
                self._status = ActionStatus.CANCELING

        self._cancel_goal(goal_handle)
        return True

    def add_done_callback(self, callback) -> None:
        with self._lock:
            if not self._event.is_set():
                self._done_callbacks.append(callback)
                return

        callback(self)

    def _on_goal_response(self, future: Future) -> None:
        try:
            goal_handle = future.result()
        except Exception as error:
            self._complete(ActionStatus.ABORTED, message=str(error))
            return

        if not goal_handle.accepted:
            self._complete(ActionStatus.REJECTED, message="Goal rejected")
            return

        with self._lock:
            self._goal_handle = goal_handle
            cancel_requested = self._cancel_requested
            self._status = (
                ActionStatus.CANCELING if cancel_requested else ActionStatus.ACTIVE
            )

        try:
            get_result = goal_handle.get_result_async()
        except Exception as error:
            self._complete(ActionStatus.ABORTED, message=str(error))
            return

        get_result.add_done_callback(self._on_result)
        if cancel_requested:
            self._cancel_goal(goal_handle)

    def _on_result(self, future: Future) -> None:
        try:
            response = future.result()
        except Exception as error:
            self._complete(ActionStatus.ABORTED, message=str(error))
            return

        self._complete(
            _GOAL_STATUS_MAP.get(response.status, ActionStatus.ABORTED),
            result=response.result,
            message=getattr(response.result, "message", ""),
        )

    def _cancel_goal(self, goal_handle: Any | None) -> None:
        if goal_handle is None:
            return

        with self._lock:
            if self._cancel_sent or self._event.is_set():
                return
            self._cancel_sent = True

        try:
            goal_handle.cancel_goal_async()
        except Exception as error:
            self._complete(ActionStatus.ABORTED, message=str(error))

    def _complete(
        self, status: ActionStatus, result: Any = None, message: str = ""
    ) -> None:
        with self._lock:
            if self._event.is_set():
                return

            self._status = status
            self._result = result
            self._message = message
            callbacks = self._done_callbacks
            self._done_callbacks = []

        self._event.set()
        for callback in callbacks:
            callback(self)