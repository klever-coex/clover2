import argparse
import logging
import pathlib
import re
import sys

from version_manager.commands import COMMANDS
from version_manager.stores import discover_stores

logger = logging.getLogger(__name__)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Project version manager"
    )

    parser.add_argument(
        "-d",
        "--dir",
        required=True,
        type=pathlib.Path,
        help="Dir for version stores search"
    )
    parser.add_argument(
        "-f",
        "--filter",
        type=re.compile,
        default=re.compile(".*"),
        help="Package name filter"
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase output verbosity"
    )

    subparsers = parser.add_subparsers(
        dest="command",
        help="Available subcommands",
        required=True
    )

    for name, command_cls in sorted(COMMANDS.items()):
        command = command_cls()
        sub = subparsers.add_parser(name, help=command.help)
        command.configure_parser(sub)
        sub.set_defaults(command_cls=command_cls)

    return parser


def setup_logging(verbose: int) -> None:
    if verbose >= 3:
        level = logging.DEBUG
    elif verbose == 2:
        level = logging.INFO
    elif verbose == 1:
        level = logging.WARNING
    else:
        level = logging.ERROR

    logging.basicConfig(
        level=level,
        format="[%(levelname)s][%(asctime)s]: %(name)s: %(message)s",
    )


def main() -> None:
    args = build_parser().parse_args()
    setup_logging(args.verbose)

    stores = discover_stores(args.dir, args.filter)
    if not stores:
        logger.error("No version stores found in %s", args.dir)
        sys.exit(1)

    args.command_cls().run(args, stores, args.dir)
