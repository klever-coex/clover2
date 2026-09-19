import argparse

from clover2_tooling.commands.base import Group
from clover2_tooling.builder.config import image_configurations
from clover2_tooling.commands.builder import build, download, images, setup, upload


class BuilderCommand(Group):
    name = "builder"
    help = "Disk image builder"

    children = (
        download.DownloadCommand,
        images.ImagesCommand,
        build.BuildCommand,
        setup.SetupCommand,
        upload.UploadCommand,
    )

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "-c", "--configuration",
            default="klever5",
            choices=list(image_configurations),
            help="Image configuration declaring docker_images"
        )

        super().configure_parser(parser)
