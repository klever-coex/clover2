import argparse

from clover2_tooling.version import versioning
from clover2_tooling.commands.base import Command
from clover2_tooling.version.stores import discover_stores


class ComposeCommand(Command):
    name = "compose"
    help = "Compose the full version from the git context"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "--ref",
            help="Git ref: refs/tags/vX, refs/heads/... or bare vX tag"
        )
        parser.add_argument(
            "--mode",
            choices=["develop", "master", "release", "pre-release"],
            help="Explicit build mode (local builds); otherwise derived from --ref"
        )
        parser.add_argument(
            "--latest-rc",
            action="store_true",
            help="Report the newest rc tag (for promote)"
        )
        parser.add_argument(
            "--latest-stable",
            action="store_true",
            help="Report the newest stable tag (for changelog ranges)"
        )
        self.add_output_group(parser)

    def run(self, args: argparse.Namespace) -> None:
        stores = discover_stores(args.dir, args.filter)

        payload = versioning.compose(stores, args.dir, ref=args.ref, mode=args.mode,
                                     latest_rc=args.latest_rc,
                                     latest_stable=args.latest_stable)

        self.emit(payload, args)
