import pathlib
from datetime import datetime

PROJECT_DIR = pathlib.Path(__file__).absolute().parent

project = "clover2"
author = "Lapin Matvey"
copyright = f"{datetime.now().year}, Lapin Matvey"

extensions = [
    "myst_parser",
    "sphinx_design",
    "sphinx.ext.autodoc",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
    "sphinx.ext.autosectionlabel",
    "sphinx_copybutton",
]

autosectionlabel_prefix_document = True

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "html_image",
]

source_suffix = {
    ".md": "markdown",
}

templates_path = [
    (PROJECT_DIR / "templates").as_posix(),
]

exclude_patterns = [
    "build",
    "Thumbs.db",
    ".DS_Store",
    "**/_*.rst",
]

master_doc = "index"

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "collapse_navigation": False,
    "sticky_navigation": False,
    "navigation_depth": -1,
}

html_static_path = ["assets"]


def resolve_assets_ref(app, docname, source):
    """Replace @assets@ with correct relative path from current file to assets/."""
    depth = len(docname.split("/"))
    source[0] = source[0].replace("@assets@", "../" * depth + "assets")


def setup(app):
    app.connect("source-read", resolve_assets_ref)
