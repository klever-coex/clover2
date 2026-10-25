"""Future waiting helper."""

import threading
from typing import Any

from rclpy.task import Future


def wait_future(future: Future, timeout: float) -> Any:
    """Wait for a future and return its result."""
    event = threading.Event()

    def unblock(future: Future):
        nonlocal event
        event.set()

    future.add_done_callback(unblock)

    if not future.done():
        event.wait(timeout)

    if future.exception() is not None:
        raise future.exception()

    return future.result()
