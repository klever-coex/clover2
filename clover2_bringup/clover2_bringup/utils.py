"""Bringup helpers."""

from collections.abc import Sequence
from dataclasses import is_dataclass
from typing import Any, get_type_hints


def from_dict(cls, data: dict):
    """Build a dataclass instance from a dict."""
    if not isinstance(data, dict):
        return data
    hints = get_type_hints(cls)
    kwargs = {}
    for key, value in data.items():
        field_type = hints.get(key)
        if field_type and is_dataclass(field_type) and isinstance(value, dict):
            kwargs[key] = from_dict(field_type, value)
        else:
            kwargs[key] = value
    return cls(**kwargs)


def resolve_params_files(context) -> list:
    """Resolve the params_files launch argument."""
    from launch.substitutions import LaunchConfiguration

    raw = LaunchConfiguration("params_files").perform(context)
    return [f for f in raw.split() if f]


def _ns_segments(ns: Any) -> Sequence[str]:
    if ns is None:
        return []
    if isinstance(ns, str):
        return [s for s in ns.split("/") if s]
    if isinstance(ns, (list, tuple)):
        return [str(p).strip("/") for p in ns if p]
    return []


def parent_namespace(ns: Any, name: str | None = None) -> str:
    """Return the parent namespace of a node."""
    segs = _ns_segments(ns)
    if name and segs and segs[-1] == name:
        segs = segs[:-1]
    return "/" + "/".join(segs) if segs else "/"


def join_fqn(namespace: str, node_name: str) -> str:
    """Join a namespace and node name into a full name."""
    ns = namespace if namespace.startswith("/") else "/" + namespace
    ns = ns.rstrip("/")
    if not node_name:
        return ns or "/"
    if node_name.startswith("/"):
        return node_name
    return f"{ns}/{node_name}" if ns else f"/{node_name}"
