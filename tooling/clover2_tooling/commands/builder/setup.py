import argparse
import logging
import os
import subprocess
import pathlib

from clover2_tooling.commands.base import Command

logger = logging.getLogger(__name__)

RUNNER_PATH = pathlib.Path(
    __file__).parent.parent.parent / "builder" / "image-setup.sh"


class SetupCommand(Command):
    name = "setup"
    help = "Run the stage runner locally (intended to run inside the image)"

    def configure_parser(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            "--list",
            action="store_true",
            help="List stages and exit"
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

    def run(self, args: argparse.Namespace) -> None:
        passthrough = []
        if args.list:
            passthrough.append("--list")
        if args.stages:
            passthrough += ["--stages", args.stages]
        if args.skip:
            passthrough += ["--skip", args.skip]
        if args.fresh:
            passthrough.append("--fresh")

        env = {
            **os.environ,
            "REGISTRY": os.environ.get("REGISTRY", ""),
            "CLOVER2_VERSION": os.environ.get("CLOVER2_VERSION", ""),
            "CLOVER2_GIT_HASH": os.environ.get("CLOVER2_GIT_HASH", ""),
        }

        logger.info("Running stage runner: %s", " ".join(
            passthrough) or "(all pending)")
        subprocess.run(["/bin/bash", str(RUNNER_PATH), *
                       passthrough], check=True, env=env)
