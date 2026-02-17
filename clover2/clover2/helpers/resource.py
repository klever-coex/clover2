
import pathlib

CLOVER2_RESOURCE_DIR = pathlib.Path('/opt/clover2')

def find_file(filename: str | pathlib.Path, dirs: list[pathlib.Path]) -> pathlib.Path | None:
    for d in dirs:
        f = d / filename
        if f.exists():
            return f
    return None
