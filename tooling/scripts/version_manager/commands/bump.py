import argparse
import pathlib

from version_manager import versioning
from version_manager.commands.base import Command


class BumpCommand(Command):
    name = "bump"
    help = "Bump reference package version and sync all stores"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "field",
            choices=["major", "minor", "patch", "rc"],
            help="Version field to bump ('rc' manages the prerelease tag series)"
        )
        parser.add_argument(
            "--base",
            choices=["major", "minor", "patch"],
            default="minor",
            help="Base bump applied when cutting a fresh rc series"
        )
        self.add_output_group(parser)

    def run(self, args: argparse.Namespace, stores, base_path: pathlib.Path) -> None:
        if args.field == "rc":
            payload = versioning.bump_rc(stores, base_path, args.base)
        else:
            payload = versioning.bump(stores, args.field)

        self.emit(payload, args)
