import argparse
import logging
import pathlib

from clover2_tooling.builder import build as build_flow, docker_images
from clover2_tooling.builder.config import get_configuration
from clover2_tooling.builder.settings import add_builder_args, settings_from_args
from clover2_tooling.commands.base import Command
from clover2_tooling.commands.builder.download import default_output
from clover2_tooling.commands.builder.images import default_images_dir

logger = logging.getLogger(__name__)

STAGES_DIR = pathlib.Path(__file__).parent.parent.parent / "builder" / "stages"


def stages_args(args: argparse.Namespace) -> str:
    parts = []
    if args.stages:
        parts.append(f"--stages {args.stages}")
    if args.skip:
        parts.append(f"--skip {args.skip}")
    if args.fresh:
        parts.append("--fresh")
    return " ".join(parts)


class BuildCommand(Command):
    name = "build"
    help = "Build the clover2 disk image"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        add_builder_args(parser)
        parser.add_argument(
            "-o", "--output",
            type=pathlib.Path,
            help="Output .img path (default: build-<cfg>/clover2-<version>.img)"
        )
        parser.add_argument(
            "--stages",
            help="Comma-separated stage names to run (default: all pending)"
        )
        parser.add_argument(
            "--skip",
            help="Comma-separated stage names to skip"
        )
        parser.add_argument(
            "--fresh",
            action="store_true",
            help="Run all stages from scratch, ignoring completion markers"
        )
        parser.add_argument(
            "--skip-images",
            action="store_true",
            help="Do not pull/save the declared docker images"
        )
        parser.add_argument(
            "--list",
            action="store_true",
            help="List stages and exit"
        )
        parser.add_argument(
            "--dry-run",
            action="store_true",
            help="Resolve the configuration and print the plan without building"
        )

    def run(self, args: argparse.Namespace) -> None:
        if args.list:
            for stage in sorted(STAGES_DIR.glob("*.sh")):
                print(stage.name)
            return

        cfg = get_configuration(args.configuration)
        settings = settings_from_args(args)
        payload = settings.composed_version()
        output = args.output or default_output(
            settings, cfg.name, payload["version"])
        tars_dir = default_images_dir(settings, cfg.name)

        if args.dry_run:
            print(f"configuration: {cfg.name}")
            print(f"build mode: {settings.build_mode}")
            print(
                f"version: {payload['version']} ({settings.artifact_tag(payload)})")
            print(f"output: {output}")
            print(
                f"docker images: {', '.join(cfg.docker_images) or '-'} -> {tars_dir}")
            print(f"compression: {cfg.compression}")
            print(
                f"stages: {'custom' if (args.stages or args.skip) else 'all pending'}")
            print(f"stages args: {stages_args(args) or '-'}")
            return

        if not output.is_file():
            raise RuntimeError(
                f"Base image '{output}' not found; run 'clover2 builder download' first")

        logger.info("Loading docker images")
        tars: list[pathlib.Path] = []
        if cfg.docker_images and not args.skip_images:
            tars = docker_images.fetch(
                cfg.docker_images, settings.registry, settings.artifact_tag(
                    payload),
                f"linux/{cfg.arch}", tars_dir)
        elif cfg.docker_images:
            tars = sorted(tars_dir.glob("*.tar"))
            logger.info("Skipping image pull, using existing tars: %s",
                        ", ".join(t.name for t in tars) or "-")

        logger.info("Building '%s' (version %s)", cfg.name, payload["version"])
        build_flow.build(settings, payload, output,
                         tars, stages_args(args))
        logger.info("Build finished: %s", output)
