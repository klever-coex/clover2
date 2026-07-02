import pathlib

CLOVER2_RESOURCE_DIR = pathlib.Path("/opt/clover2")
CLOVER2_EXTRA_DIRS = [
    pathlib.Path.home() / "clover2",
    CLOVER2_RESOURCE_DIR,
]
