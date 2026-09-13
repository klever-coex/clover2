import argparse
import pathlib

from version_manager.commands.base import Command
from version_manager.stores import reference_store, VersionStore


class PrintCommand(Command):
    name = "print"
    help = "Print stores versions"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "--main-only",
            action="store_true",
            help="Print only reference package version"
        )

    def run(self, args: argparse.Namespace, stores: list[VersionStore], base_path: pathlib.Path) -> None:
        if args.main_only:
            print(reference_store(stores).read())
            return

        all_versions: dict[str, list[str]] = {}

        for store in stores:
            if not all_versions.get(store.store_name):
                all_versions[store.store_name] = []

            all_versions[store.store_name] += [f"{store.name}: {store.read()}"]

        for store, versions in all_versions.items():
            print(f"{store}:")
            for v in versions:
                print(f"\t{v}")
