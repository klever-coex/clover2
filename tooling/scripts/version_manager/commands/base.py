import argparse
import abc
import json
import logging
import pathlib
import sys

logger = logging.getLogger(__name__)

COMMANDS: dict[str, type["Command"]] = {}


class Command(abc.ABC):

    name: str = ""
    help: str = ""

    def __init_subclass__(cls, **kwargs) -> None:
        super().__init_subclass__(**kwargs)
        if not cls.name:
            return

        if cls.name in COMMANDS:
            raise RuntimeError(f"Duplicate command name '{cls.name}'")

        COMMANDS[cls.name] = cls

    @abc.abstractmethod
    def configure_parser(self, parser: argparse.ArgumentParser) -> None: ...

    @abc.abstractmethod
    def run(self, args: argparse.Namespace, stores,
            base_path: pathlib.Path) -> None: ...

    @staticmethod
    def add_output_group(parser: argparse.ArgumentParser) -> None:
        group = parser.add_mutually_exclusive_group()

        group.add_argument("--json", action="store_true",
                           help="Emit machine-readable JSON")
        group.add_argument("--field", help="Print a single payload field")

    def emit(self, payload: dict, args: argparse.Namespace) -> None:
        if args.json:
            print(json.dumps(payload))
            return

        if args.field:
            if args.field not in payload:
                logger.error("Unknown field '%s'; available: %s",
                             args.field, ", ".join(payload))
                sys.exit(1)

            print(payload[args.field])

            return

        for key, value in payload.items():
            print(f"{key}: {value}")
