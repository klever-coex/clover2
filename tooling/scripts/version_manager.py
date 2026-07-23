#!/usr/bin/env python3

import argparse
import os
import logging
import re
import pathlib
import semver
import xml.etree.ElementTree as ET

logger = logging.getLogger("patcher")


class Package:
    def __init__(self, xml: pathlib.Path):

        self.__xml = xml
        self.__xml_obj = ET.parse(xml)

        root = self.__xml_obj.getroot()
        self.__name = root.find("name")
        self.__version = root.find("version")

    def save(self):
        self.__xml_obj.write(self.__xml, encoding="utf-8",
                             xml_declaration=True)

    @property
    def path(self) -> pathlib.Path:
        return self.__xml.parent

    @property
    def name(self) -> str:
        return self.__name.text

    @property
    def version(self) -> semver.Version:
        return self.__version.text

    @version.setter
    def version(self, value: str | semver.Version):
        val = semver.Version.parse(value)
        self.__version.text = str(val)


def build_package_list(base_path: pathlib.Path, filter: re.Pattern) -> list[Package]:
    for entry in base_path.rglob("package.xml"):
        print(entry)


def setup_logging(verbose: bool) -> None:
    level = logging.DEBUG if verbose else logging.INFO

    logging.basicConfig(
        level=level,
        format="%(asctime)s [%(levelname)s] %(message)s",
        datefmt="%H:%M:%S",
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="ROS2 package version manager"
    )

    parser.add_argument("-f", "--filter", type=re.compile,
                        help="Package name filter")

    return parser.parse_args()


def main():
    args = parse_args()
    setup_logging(args.verbose)


if __name__ == "__main__":
    main()

    setup_logging(True)

    build_package_list(pathlib.Path(
        "/home/motya/own_projects/coex/clover2-dev/src"), "clover2*")

    # val = Package(
    #     "/home/motya/own_projects/coex/clover2-dev/src/clover2/clover2/package.xml")
