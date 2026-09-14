import argparse
import pathlib
import re

from clover2_tooling.commands.base import Group
from clover2_tooling.commands.version import bump, compose, show, update


class VersionCommand(Group):
    name = "version"
    help = "Project version management"

    children = (
        show.ShowCommand,
        update.UpdateCommand,
        bump.BumpCommand,
        compose.ComposeCommand,
    )

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "-d",
            "--dir",
            type=pathlib.Path,
            default=pathlib.Path("."),
            help="Dir for version stores search"
        )
        parser.add_argument(
            "-f",
            "--filter",
            type=re.compile,
            default=re.compile(".*"),
            help="Package name filter"
        )

        super().configure_parser(parser)
