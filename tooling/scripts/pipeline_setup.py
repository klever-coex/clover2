#!/usr/bin/env python3

import argparse
import dataclasses
import typing as tp
import logging
import os
import json
import git

import config


logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def change_version_to_commit():
    # Reading current commit head
    repo = git.Repo(config.REPO_DIR)
    short_hash = repo.head.object.hexsha[:8]

    # Setting new version
    current_version = bic.read_current_version(config.REPO_DIR)

    # Replacing current buildmetadata (probably 'dirty')
    current_version.buildmetadata = f"{short_hash}"

    logger.info("Changed version to '%s'", current_version)
    bic.write_current_version(config.REPO_DIR, current_version)


@dataclasses.dataclass()
class ModeDescription:
    operations: tp.List[tp.Callable[[], None]]


modes = {
    "develop": ModeDescription(
        operations=[
            change_version_to_commit,
        ],
    ),
    "master": ModeDescription(
        operations=[
            change_version_to_commit,
        ],
    ),
    "release": ModeDescription(
        operations=[],
    ),
}


def parse_args():
    args = argparse.ArgumentParser()

    args.add_argument("--mode", "-m", choices=list(modes.keys()))

    return args.parse_args()


def main(args):
    logger.info("Activating '%s' build mode", args.mode)
    new_mode = modes[args.mode]

    logger.info("Applying operations")
    for operation in new_mode.operations:
        logger.info("Applying operation '%s'", operation.__name__)
        try:
            operation()
        except:
            logger.error(
                "Unable to apply operation '%s' for mode '%s'",
                str(operation),
                new_mode,
                exc_info=True,
            )

            raise


if __name__ == "__main__":
    main(parse_args())
