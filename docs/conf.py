import os
import pathlib
from datetime import date, datetime

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
    "sphinxcontrib.mermaid",
    "sphinx_copybutton",
    "sphinx_simplepdf",
    "breathe",
]

breathe_projects = {
    "clover2": "./build/doxygen/xml"
}

breathe_default_project = "clover2"

pygments_style = 'sphinx'
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
    "Events/TemplateEvents.md",
]

master_doc = "index"

html_theme = "furo"
html_sidebars = {
    '**': [
        "sidebar/brand.html",
        "sidebar/search.html",
        "sidebar/scroll-start.html",
        "sidebar/navigation.html",
        "sidebar/ethical-ads.html",
        "sidebar/scroll-end.html",
        "sidebar/variant-selector.html",
        "language-selector.html",
    ]
}

html_theme_options = {
    "light_css_variables": {
        "color-brand-primary": "#fb4616",
        # "color-brand-content": "#CC3333",
    },
    "dark_css_variables": {
        "color-brand-primary": "#fb4616",
        # "color-brand-content": "#CC3333",
    },
    "source_repository": "https://github.com/klever-coex/clover2/",
    "source_branch": "master",
    "sidebar_hide_name": True,
}

html_static_path = ["assets"]

html_show_sourcelink = True
html_favicon = "assets/coex.svg"

LANGUAGES = [
    ("ru", "Русский"),
    ("en", "English"),
]

def setup(app):
    def on_config_inited(app, config):
        lang = config.language or "ru"

        config.html_theme_options["source_directory"] = f"docs/{lang}/"

        other_languages = []
        for code, name in LANGUAGES:
            if code == lang:
                continue
            src_dir = PROJECT_DIR / code
            pages = {
                p.relative_to(src_dir).with_suffix("").as_posix()
                for p in src_dir.rglob("*.md")
                if not p.name.startswith("_")
            }
            other_languages.append({"code": code, "name": name, "pages": pages})

        config.html_context["lang_selector"] = {
            "title": "Язык" if lang == "ru" else "Language",
            "languages": [
                {"code": code, "name": name}
                for code, name in LANGUAGES
                if code == lang
            ]
            + other_languages,
        }

    def on_source_read(app, docname, source):
        depth = len(docname.split("/"))
        source[0] = source[0].replace("@assets@", "../" * depth + "assets")

    app.connect("config-inited", on_config_inited)
    app.connect("source-read", on_source_read)
