import json
import pathlib
import re

import semver

from version_manager.stores.base import VersionStore, bare


class NpmPackageStore(VersionStore):
    FILENAME = "package.json"
    STORE_NAME: str = "npm_package"

    VERSION_RE = re.compile(r'("version"\s*:\s*")([^"]*)(")')

    def __init__(self, path: pathlib.Path):
        super().__init__(path)
        self._name = json.loads(self.path.read_text()).get(
            "name", self.path.parent.name)

    @property
    def name(self) -> str:
        return self._name

    def read(self) -> semver.Version:
        match = self.VERSION_RE.search(self.path.read_text())

        if not match:
            raise RuntimeError(f"No version field in {self.path}")

        return semver.Version.parse(match.group(2))

    def write(self, version: semver.Version) -> None:
        text = self.path.read_text()
        new_text, count = self.VERSION_RE.subn(
            rf"\g<1>{bare(version)}\g<3>", text, count=1)

        if count != 1:
            raise RuntimeError(f"No version field in {self.path}")

        self.path.write_text(new_text)
