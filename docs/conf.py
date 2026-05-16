import os
import pathlib

PROJECT_DIR = pathlib.Path(__file__).absolute().parent

project = "clover2"
author = "Lapin Matvey"
copyright = "2025, Lapin Matvey"
version = os.environ["CLOVER2_VERSION"] or "unknown"

extensions = [
    "myst_parser",
    "sphinx_design",
    "sphinx.ext.autodoc",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
    "sphinx.ext.autosectionlabel",
    "sphinx_copybutton",
    "sphinx_simplepdf"
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

html_context = {

}

html_static_path = [
    (PROJECT_DIR / "assets").as_posix(),
]
