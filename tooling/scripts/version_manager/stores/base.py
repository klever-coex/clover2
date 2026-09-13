import abc
import logging
import os
import pathlib
import re
import sys

import semver

logger = logging.getLogger(__name__)

STORES: dict[str, type["VersionStore"]] = {}

REFERENCE_PACKAGE = "clover2"
SKIP_DIRS = {"node_modules", ".git", "build", "dist"}


class VersionStore(abc.ABC):
    FILENAME: str = ""
    STORE_NAME: str = ""
    CAN_BE_REFERENCE = False

    def __init__(self, path: pathlib.Path):
        self.path = path

    def __init_subclass__(cls, **kwargs) -> None:
        super().__init_subclass__(**kwargs)

        if not cls.FILENAME:
            return

        if cls.FILENAME in STORES:
            raise RuntimeError(f"Duplicate store filename '{cls.FILENAME}'")

        STORES[cls.FILENAME] = cls

    @property
    def store_name(self) -> str:
        return self.STORE_NAME

    @property
    @abc.abstractmethod
    def name(self) -> str: ...

    @abc.abstractmethod
    def read(self) -> semver.Version: ...

    @abc.abstractmethod
    def write(self, version: semver.Version) -> None: ...


def bare(version: semver.Version) -> semver.Version:
    if version.prerelease is not None or version.build is not None:
        logger.warning(
            "Version %s carries a suffix; stores keep bare versions only", version)

    return version.finalize_version()


def create_store(path: pathlib.Path) -> VersionStore | None:
    store_cls = STORES.get(path.name)
    return store_cls(path) if store_cls else None


def discover_stores(base_path: pathlib.Path, name_filter: re.Pattern) -> list[VersionStore]:
    stores = []

    for root, dirs, files in os.walk(base_path):
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
        for file in files:
            store = create_store(pathlib.Path(root) / file)
            if store is not None and name_filter.search(store.name):
                logger.debug("Found store: %s of %s (%s)", store.name, store.store_name, store.read())
                stores.append(store)

    return stores


def reference_store(stores: list[VersionStore]) -> VersionStore:
    for store in stores:
        if store.name == REFERENCE_PACKAGE and store.CAN_BE_REFERENCE:
            return store

    logger.error("Reference package '%s' not found", REFERENCE_PACKAGE)
    sys.exit(1)


def write_all(stores: list[VersionStore], version: semver.Version) -> None:
    for store in stores:
        old = store.read()
        store.write(version)
        logger.info("Updated %s: %s -> %s", store.name, old, bare(version))
