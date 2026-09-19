import argparse
import logging
import pathlib

from clover2_tooling.builder import compress, minio
from clover2_tooling.builder.config import get_configuration
from clover2_tooling.builder.settings import add_builder_args, settings_from_args
from clover2_tooling.commands.base import Command
from clover2_tooling.commands.builder.download import default_output

logger = logging.getLogger(__name__)


class UploadCommand(Command):
    name = "upload"
    help = "Compress the image (format from config) and upload it to MinIO"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        add_builder_args(parser)
        parser.add_argument(
            "-i", "--image",
            type=pathlib.Path,
            help="Path to the .img (default: build-<cfg>/clover2-<version>.img)"
        )

    def run(self, args: argparse.Namespace) -> None:
        cfg = get_configuration(args.configuration)
        settings = settings_from_args(args)
        payload = settings.composed_version()

        image = args.image or default_output(
            settings, cfg.name, payload["version"])
        if not image.is_file():
            raise RuntimeError(f"Image '{image}' not found; build it first")

        artifact = compress.compress(image, cfg.compression, image.parent)
        channel = "release" if settings.build_mode == "release" else "develop"

        key = minio.upload(artifact, cfg, channel, settings)
        print(
            f"{settings.minio_endpoint}/{minio.BUCKET}/{key}" if settings.minio_endpoint else key)
