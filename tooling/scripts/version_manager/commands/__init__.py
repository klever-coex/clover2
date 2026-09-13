import importlib
import pkgutil

from version_manager.commands.base import COMMANDS, Command

for _module in pkgutil.iter_modules(__path__):
    if not _module.name.startswith("_"):
        importlib.import_module(f"{__name__}.{_module.name}")

__all__ = ["COMMANDS", "Command"]
