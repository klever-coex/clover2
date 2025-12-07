#!/usr/bin/env python3

import argparse
import logging
from pathlib import Path
import yaml
import sys

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

SERVICES = {
    "clover2-docs": {
        "ports": ["80:81"],
    },
    "clover2-gui": {
        "ports": ["80:80"],
    },
    "clover2-wetty": {
        "ports": ["3000:3000"],
        "image": "wettyoss/wetty",
    },
}


def build_service_entry(name: str, cfg: dict, registry: str, tag: str, project_root: Path):

    if cfg.get("image"):
        image = cfg["image"]
    else:
        image = f"{registry.rstrip('/')}/{name}:{tag}"

    service = {
        "image": image,
        "restart": "unless-stopped",
    }

    if cfg.get("ports"):
        service["ports"] = cfg["ports"]

    if cfg.get("ipc"):
        service["ipc"] = "host"

    return service


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate a docker-compose.yaml for selected services."
    )

    group = parser.add_argument_group("services", "Enable/disable services")
    for svc in SERVICES.keys():
        group.add_argument(
            f"--{svc}",
            action="store_true",
            help=f"Include the '{svc}' service (config from SERVICES dict).",
        )

    parser.add_argument(
        "--output",
        "-o",
        type=Path,
        default=Path.cwd() / "docker-compose.yaml",
        help="Output path for generated docker-compose.yaml",
    )
    parser.add_argument(
        "--version",
        "-v",
        dest="version",
        default="latest",
        help="Image tag/version to use for services (image tag)",
    )
    parser.add_argument(
        "--registry",
        "-r",
        default="registry.gitlab.com/coex2/clover2",
        help="Registry prefix to use for images",
    )
    parser.add_argument(
        "--project-root",
        type=Path,
        default=Path.cwd(),
        help="Project root used to place host-side log directories (default: current working dir)",
    )

    return parser.parse_args()


def main():
    args = parse_args()

    selected = [name for name in SERVICES.keys(
    ) if getattr(args, name.replace("-", "_"))]
    if not selected:
        logger.error(
            "No services selected. Pass one or more service flags.")
        sys.exit(2)

    compose = {"version": "3.8", "services": {}}

    project_root = args.project_root.resolve()

    for svc in selected:
        cfg = SERVICES[svc]
        compose["services"][svc] = build_service_entry(
            svc, cfg, registry=args.registry, tag=args.version, project_root=project_root
        )

    out_path: Path = args.output
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", encoding="utf-8") as f:
        yaml.safe_dump(compose, f, sort_keys=False, default_flow_style=False)

    logger.info("Wrote docker-compose to %s", out_path)


if __name__ == "__main__":
    main()
