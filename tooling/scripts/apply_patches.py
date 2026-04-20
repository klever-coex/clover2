#!/usr/bin/env python3
import argparse
import logging
import os
import subprocess
import sys
from typing import List

logger = logging.getLogger("patcher")


def setup_logging(verbose: bool) -> None:
    level = logging.DEBUG if verbose else logging.INFO

    logging.basicConfig(
        level=level,
        format="%(asctime)s [%(levelname)s] %(message)s",
        datefmt="%H:%M:%S",
    )


def run_command(
    cmd: List[str], cwd: str, check: bool = True
) -> subprocess.CompletedProcess:
    logger.debug(f"Running command: {' '.join(cmd)} (cwd={cwd})")

    result = subprocess.run(cmd, cwd=cwd)

    if check and result.returncode != 0:
        logger.error(f"Command failed: {' '.join(cmd)}")
        raise RuntimeError(f"Command failed: {' '.join(cmd)}")

    return result


def ensure_git_repo(repo_path: str) -> None:
    if not os.path.isdir(os.path.join(repo_path, ".git")):
        logger.info("Initializing git repository")
        run_command(["git", "init"], cwd=repo_path)
    else:
        logger.debug("Git repository already exists")


def collect_patches(patches_path: str, recursive: bool = True) -> List[str]:
    patch_files = []

    if recursive:
        for root, _, files in os.walk(patches_path):
            for f in files:
                if f.endswith((".patch", ".diff")):
                    patch_files.append(os.path.join(root, f))
    else:
        for f in os.listdir(patches_path):
            if f.endswith((".patch", ".diff")):
                patch_files.append(os.path.join(patches_path, f))

    patch_files = sorted(patch_files)

    logger.info(f"Found {len(patch_files)} patch(es)")
    logger.debug(f"Patches list: {patch_files}")

    return patch_files


def apply_patch(repo_path: str, patch_file: str) -> None:
    logger.info(f"Applying patch: {patch_file}")

    apply_cmd = ["git", "apply"]

    check_cmd = apply_cmd + ["--check", patch_file]
    check_result = subprocess.run(check_cmd, cwd=repo_path)

    if check_result.returncode == 0:
        run_command(apply_cmd + [patch_file], cwd=repo_path)
        logger.debug("Applied via git apply")
        return

    logger.warning("git apply failed, trying git am")

    try:
        run_command(["git", "am", patch_file], cwd=repo_path)
        logger.debug("Applied via git am")
    except RuntimeError:
        logger.error("git am failed, aborting sequence")
        subprocess.run(["git", "am", "--abort"], cwd=repo_path)
        raise


def apply_all_patches(repo_path: str, patches: List[str]) -> None:
    if not patches:
        logger.warning("No patches found")
        return

    for patch in patches:
        apply_patch(repo_path, patch)

    logger.info("All patches applied successfully")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Apply .patch/.diff files to a git repository"
    )

    parser.add_argument("repo", help="Path to project repository")
    parser.add_argument("patches", help="Path to directory with patches")

    parser.add_argument(
        "--recursive", action="store_true", help="Search patches recursively"
    )

    parser.add_argument("--verbose", action="store_true", help="Enable debug logging")

    return parser.parse_args()


def validate_paths(repo_path: str, patches_path: str) -> None:
    if not os.path.isdir(repo_path):
        raise ValueError(f"Repository path not found: {repo_path}")

    if not os.path.isdir(patches_path):
        raise ValueError(f"Patches path not found: {patches_path}")


def main():
    args = parse_args()
    setup_logging(args.verbose)

    repo_path = os.path.abspath(args.repo)
    patches_path = os.path.abspath(args.patches)

    try:
        validate_paths(repo_path, patches_path)
        ensure_git_repo(repo_path)

        patches = collect_patches(patches_path, recursive=args.recursive)

        apply_all_patches(repo_path, patches)

    except Exception as e:
        logger.exception(f"Execution failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
