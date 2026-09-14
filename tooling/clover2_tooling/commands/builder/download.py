import argparse
import logging
import pathlib

from clover2_tooling.builder import base_image
from clover2_tooling.builder.config import get_configuration
from clover2_tooling.builder.settings import add_builder_args, settings_from_args
from clover2_tooling.commands.base import Command

logger = logging.getLogger(__name__)


def default_output(settings, cfg_name: str, version: str) -> pathlib.Path:
    return settings.project_dir / f"build-{cfg_name}-image" / f"{cfg_name}-{version}.img"


class DownloadCommand(Command):
    name = "download"
    help = "Download and prepare the base disk image"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        add_builder_args(parser)
        parser.add_argument(
            "-o", "--output",
            type=pathlib.Path,
            help="Output .img path (default: build-<cfg>/<cfg_name>-<version>.img)"
        )

    def run(self, args: argparse.Namespace) -> None:
        cfg = get_configuration(args.configuration)
        settings = settings_from_args(args)
        payload = settings.composed_version()

        output = args.output or default_output(
            settings, cfg.name, payload["version"])
        logger.info("Configuration: %s, version: %s",
                    cfg.name, payload["version"])
        base_image.download(
            cfg, output, settings.project_dir / ".cache" / "downloads")
