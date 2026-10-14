"""Common helpers for clover2 packages."""

from clover2_common._version import __version__
from clover2_common.latest_subscriber import LatestSubscriber
from clover2_common.wait_future import wait_future


def project_version() -> str:
    """Project version decided at build time (CLOVER2_VERSION env or package.xml)."""
    return __version__


__all__ = [
    "LatestSubscriber",
    "project_version",
    "wait_future",
]