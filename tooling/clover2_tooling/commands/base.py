import argparse
import abc
import json
import logging
import sys

logger = logging.getLogger(__name__)


class Command(abc.ABC):
    name: str = ""
    help: str = ""

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        """Register command-specific arguments."""

    @abc.abstractmethod
    def run(self, args: argparse.Namespace) -> None: ...

    @staticmethod
    def add_output_group(parser: argparse.ArgumentParser) -> None:
        """Shared --json/--field output options for payload-emitting commands."""
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


class Group(Command):
    children: tuple[type[Command], ...] = ()

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        subparsers = parser.add_subparsers(
            dest=f"{self.name}_command", required=True, metavar="COMMAND")

        seen: set[str] = set()

        for child_cls in self.children:
            child = child_cls()
            if child.name in seen:
                raise RuntimeError(
                    f"Duplicate subcommand '{self.name} {child.name}'")

            seen.add(child.name)
            sub = subparsers.add_parser(child.name, help=child.help)
            child.configure_parser(sub)

            if not isinstance(child, Group):
                sub.set_defaults(handler=child.run)

    def run(self, args: argparse.Namespace) -> None:
        raise RuntimeError(
            f"'{self.name}' is a command group; pick a subcommand")
