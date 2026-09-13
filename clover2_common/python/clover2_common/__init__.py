from clover2_common._version import __version__


def project_version() -> str:
    """Project version decided at build time (CLOVER2_VERSION env or package.xml)."""
    return __version__
