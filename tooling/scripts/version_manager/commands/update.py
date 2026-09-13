import argparse
import logging
import pathlib
import sys

import semver

from version_manager.commands.base import Command
from version_manager.stores import write_all

logger = logging.getLogger(__name__)


class UpdateCommand(Command):
    name = "update"
    help = "Set explicit version in all stores"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "new_version",
            help="New version (suffixes are stripped: stores keep bare versions)",
            type=str
        )

    def run(self, args: argparse.Namespace, stores, base_path: pathlib.Path) -> None:
        try:
            target = semver.Version.parse(args.new_version)
        except ValueError:
            logger.error("Invalid version '%s'", args.new_version)
            sys.exit(1)

        write_all(stores, target)
