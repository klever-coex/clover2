import argparse
import logging
import pathlib

from clover2_tooling.builder import docker_images
from clover2_tooling.builder.config import get_configuration
from clover2_tooling.builder.settings import add_builder_args, settings_from_args
from clover2_tooling.commands.base import Command

logger = logging.getLogger(__name__)


def default_images_dir(settings, cfg_name: str) -> pathlib.Path:
    return settings.project_dir / f"build-{cfg_name}-image" / "docker"


class ImagesCommand(Command):
    name = "images"
    help = "Pull declared docker images (target arch only) and save as tars"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        add_builder_args(parser)
        parser.add_argument(
            "--tag",
            help="Image tag (default: hash for develop/master, version for tagged builds)"
        )
        parser.add_argument(
            "-o", "--out-dir",
            type=pathlib.Path,
            help="Dir for tars (default: build-<cfg>/docker)"
        )

    def run(self, args: argparse.Namespace) -> None:
        cfg = get_configuration(args.configuration)
        settings = settings_from_args(args)
        payload = settings.composed_version()

        if not cfg.docker_images:
            logger.info(
                "Configuration '%s' declares no docker images", cfg.name)
            return

        tag = args.tag or settings.artifact_tag(payload)
        out_dir = args.out_dir or default_images_dir(settings, cfg.name)
        logger.info("Fetching %s (tag '%s', linux/%s)",
                    ", ".join(cfg.docker_images), tag, cfg.arch)

        tars = docker_images.fetch(
            cfg.docker_images, settings.registry, tag,
            f"linux/{cfg.arch}", out_dir)
        for tar in tars:
            print(tar)
