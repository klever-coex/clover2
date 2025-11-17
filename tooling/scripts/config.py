import os
import pathlib
import glob
import typing as tp

assert os.environ.get("REPO_DIR") is not None, "REPO_DIR unset"

REPO_DIR: str = os.environ.get("REPO_DIR")

GIT_TOKEN: tp.Optional[str] = os.environ.get("GITLAB_TOKEN")
GIT_BASEURL: str = "https://gitlab.com"
