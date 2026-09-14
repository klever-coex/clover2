import argparse

from clover2_tooling.version import versioning
from clover2_tooling.commands.base import Command
from clover2_tooling.version.stores import discover_stores


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

    def run(self, args: argparse.Namespace) -> None:
        stores = discover_stores(args.dir, args.filter)

        if args.field == "rc":
            payload = versioning.bump_rc(stores, args.dir, args.base)
        else:
            payload = versioning.bump(stores, args.field)

        self.emit(payload, args)
