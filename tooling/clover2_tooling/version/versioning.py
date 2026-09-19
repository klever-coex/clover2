import logging
import pathlib

import git
import semver

from clover2_tooling.version.stores import bare, reference_store, write_all

logger = logging.getLogger(__name__)


def _open_repo(base_path: pathlib.Path) -> git.Repo | None:
    try:
        return git.Repo(base_path, search_parent_directories=True)
    except (git.InvalidGitRepositoryError, git.NoSuchPathError):
        return None


def parse_tag(tag: str) -> semver.Version:
    if not tag.startswith("v"):
        logger.error("Tag '%s' must start with 'v'", tag)
        raise SystemExit(1)

    try:
        return semver.Version.parse(tag[1:])
    except ValueError:
        logger.error("Tag '%s' is not a valid semver version", tag)
        raise SystemExit(1)


def _max_tag(tags: list[str]) -> str | None:
    candidates = []

    for tag in tags:
        try:
            candidates.append((semver.Version.parse(tag[1:]), tag))
        except ValueError:
            continue

    return max(candidates)[1] if candidates else None


def _tag_names(repo: git.Repo) -> list[str]:
    return [t.name for t in repo.tags]


def bump(stores, field: str) -> dict:
    current = reference_store(stores).read()
    target = getattr(current, f"bump_{field}")()
    write_all(stores, target)
    return {"base": str(bare(target)), "tag": f"v{bare(target)}", "stores_changed": True}


def bump_rc(stores, base_path: pathlib.Path, base_field: str) -> dict:
    current = reference_store(stores).read()

    repo = _open_repo(base_path)
    if repo is None:
        logger.warning(
            "Not a git repo; treating '%s' as a fresh rc base", current)
        rc_names, stable_exists = [], False
    else:
        names = _tag_names(repo)
        rc_names = [n for n in names if n.startswith(f"v{current}-rc.")]
        stable_exists = f"v{current}" in names

    if rc_names:
        next_rc = max(int(name.rsplit(".", 1)[1]) for name in rc_names) + 1
        return {"base": str(current), "tag": f"v{current}-rc.{next_rc}",
                "stores_changed": False}

    if stable_exists:
        target = getattr(current, f"bump_{base_field}")()
        write_all(stores, target)
        return {"base": str(bare(target)), "tag": f"v{target}-rc.1",
                "stores_changed": True}

    return {"base": str(current), "tag": f"v{current}-rc.1", "stores_changed": False}


def compose(stores, base_path: pathlib.Path, ref: str | None = None,
            mode: str | None = None, latest_rc: bool = False,
            latest_stable: bool = False) -> dict:
    repo = _open_repo(base_path)
    if repo is None:
        logger.error("'%s' is not a git repository", base_path)
        raise SystemExit(1)

    if latest_rc:
        tag = _max_tag([n for n in _tag_names(
            repo) if n.startswith("v") and "-rc." in n])
        if not tag:
            logger.error("No rc tags found")
            raise SystemExit(1)
        return {"tag": tag, "version": str(parse_tag(tag).finalize_version()),
                "commit": repo.commit(tag).hexsha}

    if latest_stable:
        tag = _max_tag([n for n in _tag_names(repo)
                        if n.startswith("v") and "-" not in n])

        if not tag:
            logger.error("No stable tags found")
            raise SystemExit(1)

        return {"tag": tag, "version": tag[1:], "commit": repo.commit(tag).hexsha}

    base = reference_store(stores).read()
    if ref and not ref.startswith("refs/"):
        if not ref.startswith("v"):
            logger.error("--ref must be a full git ref or a v-prefixed tag")
            raise SystemExit(1)

        ref = f"refs/tags/{ref}"

    if ref and ref.startswith("refs/tags/"):
        tag_name = ref[len("refs/tags/"):]
        version = parse_tag(tag_name)
        tag = next((t for t in repo.tags if t.name == tag_name), None)

        if tag is None:
            logger.error("Tag '%s' not found in repository", tag_name)
            raise SystemExit(1)

        build_mode = "pre-release" if version.prerelease else "release"
        git_hash = tag.commit.hexsha[:7]
    else:
        try:
            commit = repo.commit(ref if ref else "HEAD")
        except git.BadName:
            commit = repo.commit("HEAD")

        git_hash = commit.hexsha[:7]
        build_mode = mode

        if build_mode is None:
            build_mode = "master" if ref == "refs/heads/master" else "develop"

        version = _version_for_mode(repo, base, build_mode, git_hash)

    return {"base_version": str(base), "git_hash": git_hash,
            "build_mode": build_mode, "version": str(version)}


def _version_for_mode(repo: git.Repo, base: semver.Version,
                      mode: str, git_hash: str) -> str:
    if mode in ("develop", "master"):
        return f"{base}+{git_hash}"

    try:
        tag = repo.git.describe("--tags", "--exact-match", "HEAD")
    except git.GitCommandError:
        tag = None

    if tag:
        version = parse_tag(tag)
        if (mode == "release") == (version.prerelease is None):
            return str(version)
        logger.warning(
            "Tag '%s' does not match %s mode; using bare base version", tag, mode)
    else:
        logger.warning(
            "No tag on HEAD; %s build uses the bare base version", mode)

    return str(base)
