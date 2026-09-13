import pathlib

import semver
from lxml import etree

from version_manager.stores.base import VersionStore, bare


class RosPackageStore(VersionStore):
    FILENAME = "package.xml"
    STORE_NAME: str = "ros_package"

    def __init__(self, path: pathlib.Path):
        super().__init__(path)
        parser = etree.XMLParser(remove_blank_text=False)
        self._tree = etree.parse(self.path, parser)
        root = self._tree.getroot()
        self._name = root.findtext("name")
        self._version_el = root.find("version")

    @property
    def name(self) -> str:
        return self._name

    def read(self) -> semver.Version:
        return semver.Version.parse(self._version_el.text)

    def write(self, version: semver.Version) -> None:
        self._version_el.text = str(bare(version))
        self._tree.write(self.path, encoding="utf-8",
                         xml_declaration=True, pretty_print=True)
