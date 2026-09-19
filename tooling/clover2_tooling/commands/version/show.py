import argparse

from clover2_tooling.commands.base import Command
from clover2_tooling.version.stores import discover_stores, reference_store


class ShowCommand(Command):
    name = "show"
    help = "Show stores versions"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "--main-only",
            action="store_true",
            help="Show only reference package version"
        )

    def run(self, args: argparse.Namespace) -> None:
        stores = discover_stores(args.dir, args.filter)

        if args.main_only:
            print(reference_store(stores).read())
            return

        all_versions: dict[str, list[str]] = {}

        for store in stores:
            all_versions.setdefault(store.store_name, [])
            all_versions[store.store_name] += [f"{store.name}: {store.read()}"]

        for store_name, versions in all_versions.items():
            print(f"{store_name}:")
            for version in versions:
                print(f"\t{version}")
