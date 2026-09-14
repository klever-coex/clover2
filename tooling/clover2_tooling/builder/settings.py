import argparse
import os
import pathlib
import re
from dataclasses import dataclass

from clover2_tooling.version import versioning
from clover2_tooling.version.stores import discover_stores

DEFAULT_REGISTRY = "ghcr.io/klever-coex/clover2/"
BUILD_MODES = ("develop", "master", "release", "pre-release")


@dataclass(frozen=True)
class BuilderSettings:
    project_dir: pathlib.Path
    build_mode: str
    registry: str
    minio_endpoint: str
    minio_access_key: str
    minio_secret_key: str
    minio_secure: bool = True

    def composed_version(self) -> dict:
        stores = discover_stores(self.project_dir, re.compile(".*"))
        return versioning.compose(stores, self.project_dir, mode=self.build_mode)

    def artifact_tag(self, payload: dict) -> str:
        if self.build_mode in ("develop", "master"):
            return payload["git_hash"]
        return payload["version"]


def add_builder_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--project-dir",
        type=pathlib.Path,
        default=pathlib.Path(os.environ.get("PROJECT_DIR", os.getcwd())),
        help="Project root (env: PROJECT_DIR)"
    )
    parser.add_argument(
        "--build-mode",
        choices=BUILD_MODES,
        default=os.environ.get("BUILD_MODE", "develop"),
        help="Build mode for version composition (env: BUILD_MODE)"
    )
    parser.add_argument(
        "--registry",
        default=os.environ.get("REGISTRY", DEFAULT_REGISTRY),
        help="Docker registry prefix for baked images (env: REGISTRY)"
    )


def settings_from_args(args: argparse.Namespace) -> BuilderSettings:
    return BuilderSettings(
        project_dir=pathlib.Path(args.project_dir),
        build_mode=args.build_mode,
        registry=args.registry,
        minio_endpoint=os.environ.get("MINIO_ENDPOINT", ""),
        minio_access_key=os.environ.get("MINIO_ACCESS_KEY", ""),
        minio_secret_key=os.environ.get("MINIO_SECRET_KEY", ""),
        minio_secure=os.environ.get("MINIO_SECURE", "true").lower() != "false",
    )
