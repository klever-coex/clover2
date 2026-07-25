#!/usr/bin/env python3

import argparse
import os
import logging
import re
import pathlib
import semver
from lxml import etree

logger = logging.getLogger("patcher")


class Package:
    def __init__(self, xml: pathlib.Path):

        self.__xml = xml
        parser = etree.XMLParser(remove_blank_text=False)
        self.__xml_obj = etree.parse(self.__xml, parser)

        root = self.__xml_obj.getroot()
        self.__name = root.find("name")
        self.__version = root.find("version")

    def save(self):
        self.__xml_obj.write(self.__xml, encoding="utf-8",
                             xml_declaration=True, pretty_print=True,)

    @property
    def path(self) -> pathlib.Path:
        return self.__xml.parent

    @property
    def name(self) -> str:
        return self.__name.text

    @property
    def version(self) -> semver.Version:
        return semver.Version.parse(self.__version.text)

    @version.setter
    def version(self, value: str | semver.Version):
        if isinstance(value, semver.Version):
            self.__version.text = str(value)
        else:
            self.__version.text = str(semver.Version.parse(value))


def build_package_list(base_path: pathlib.Path, name_filter: re.Pattern) -> list[Package]:
    packages = []
    for entry in base_path.rglob("package.xml"):
        package = Package(entry)
        logger.debug(f"Probe {package.path}")
        if name_filter.search(package.name):
            packages.append(package)
    return packages


def setup_logging(verbose: bool) -> None:
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
        format="[%(levelname)s] %(asctime)s: %(message)s",
        datefmt="%H:%M:%S",
    )


def handle_print(args: argparse.Namespace, packages: list[Package]):
    if args.main_only:
        main_pkg = next((p for p in packages if p.name == "clover2"), None)
        if main_pkg:
            print(main_pkg.version)
        return

    for package in packages:
        print(f"{package.name}: {package.version}")


def handle_update(args: argparse.Namespace, packages: list[Package]):
    new_version = args.new_version

    for package in packages:
        old = str(package.version)
        package.version = new_version
        package.save()
        logger.info(f"Updated {package.name}: {old} - {package.version}")


def handle_bump(args: argparse.Namespace, packages: list[Package]):
    main_pkg = next((p for p in packages if p.name == "clover2"), None)
    if not main_pkg:
        logger.error("Main package 'clover2' not found")
        return

    old = str(main_pkg.version)
    new = getattr(main_pkg.version, f"bump_{args.field}")()
    logger.info(f"Bumping version from {old} to {new}")

    for package in packages:
        package.version = new
        package.save()
        logger.info(f"Updated {package.name}: {old} -> {package.version}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="ROS2 package version manager"
    )

    parser.add_argument(
        "-d",
        "--dir",
        required=True,
        type=pathlib.Path,
        help="Dir for package search"
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

    parser_print = subparsers.add_parser("print", help="Print packages version")
    parser_print.add_argument(
        "--main-only",
        action="store_true",
        help="Print only main package (clover2) version"
    )
    parser_print.set_defaults(func=handle_print)

    parser_update = subparsers.add_parser(
        "update", help="Update packages version")
    parser_update.add_argument(
        "new_version",
        help="New version",
        type=str
    )
    parser_update.set_defaults(func=handle_update)

    parser_bump = subparsers.add_parser("bump", help="Bump main package version")
    parser_bump.add_argument(
        "field",
        choices=["major", "minor", "patch"],
        help="Version field to bump"
    )
    parser_bump.set_defaults(func=handle_bump)

    return parser.parse_args()


def main():
    args = parse_args()
    setup_logging(args.verbose)

    packages = build_package_list(args.dir, args.filter)

    args.func(args, packages)


if __name__ == "__main__":
    main()
