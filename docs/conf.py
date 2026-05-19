import os
import pathlib

from datetime import date

PROJECT_DIR = pathlib.Path(__file__).absolute().parent

project = "clover2"
author = "Lapin Matvey"
copyright = f"{date.today().year}, Lapin Matvey"
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

html_theme = "furo"
html_theme_options = {
    # "light_css_variables": {
    #     "color-brand-primary": "orange",
    #     "color-brand-content": "#CC3333",
    # },
    # "dark_css_variables": {
    #     "color-brand-primary": "orange",
    #     "color-brand-content": "#CC3333",
    # },
    "source_repository": "https://gitlab.com/coex2/clover2/",
    "source_branch": "master",
}

html_static_path = [
    (PROJECT_DIR / "assets").as_posix(),
]

html_css_files = [
    'custom.css',
]

def setup(app):
    def on_config_inited(app, config):
        lang = config.language or "en"

        config.html_theme_options["source_directory"] = f"docs/{lang}/"

    app.connect("config-inited", on_config_inited)
