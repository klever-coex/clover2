import argparse
import logging


from clover2_tooling.commands.builder import BuilderCommand
from clover2_tooling.commands.version import VersionCommand

ROOTS = (VersionCommand, BuilderCommand)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="clover2",
        description="Clover2 project tooling: version management and image builder"
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase output verbosity"
    )

    subparsers = parser.add_subparsers(
        dest="command", required=True, metavar="COMMAND")
    for root_cls in ROOTS:
        root = root_cls()
        sub = subparsers.add_parser(root.name, help=root.help)
        root.configure_parser(sub)

    return parser


def setup_logging(verbose: int) -> None:
    if verbose >= 3:
        level = logging.DEBUG
    elif verbose == 2:
        level = logging.INFO
    elif verbose == 1:
        level = logging.WARNING
    else:
        level = logging.ERROR

    logging.basicConfig(
        level=level,
        format="[%(levelname)s] %(asctime)s: %(name)s: %(message)s",
        datefmt="%H:%M:%S",
    )


def main() -> None:
    args = build_parser().parse_args()
    setup_logging(args.verbose)
    args.handler(args)
