from version_manager.stores.base import (
    bare,
    create_store,
    discover_stores,
    reference_store,
    STORES,
    VersionStore,
    write_all,
)

import importlib
import pkgutil

for _module in pkgutil.iter_modules(__path__):
    if not _module.name.startswith("_"):
        importlib.import_module(f"{__name__}.{_module.name}")


__all__ = [
    "bare",
    "create_store",
    "discover_stores",
    "reference_store",
    "STORES",
    "VersionStore",
    "write_all",
]
