import os

TARGET_LANG = os.getenv('TARGET_LANG', 'en')

project = 'clover2'
copyright = '2025, Lapin Matvey'
author = 'Lapin Matvey'

# General configuration
extensions = [
    'myst_parser',
    'sphinx_design',
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.viewcode',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autosectionlabel',
]

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",
    "replacements",
    "smartquotes",
    "strikethrough",
    "substitution",
    "tasklist",
]

templates_path = ['_templates']

source_suffix = ['.rst', '.md']
todo_include_todos = True

# HTML config
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

languages = ['en', 'ru']

if TARGET_LANG not in languages:
    raise Exception(f'Unexpected TARGET_LANG="{TARGET_LANG}". Please select one off: {", ".join(languages)}')

root_doc = f'index_{TARGET_LANG}'

include_patterns = [
    f'{TARGET_LANG}/**',
    f'index_{TARGET_LANG}.rst'
]
