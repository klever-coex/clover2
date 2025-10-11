"""
Sphinx configuration file for the clover2 project.
Author: Lapin Matvey
Year: 2025
"""

import pathlib

# --- Project paths ---
PROJECT_DIR = pathlib.Path(__file__).absolute().parent

# --- Project information ---
project = 'clover2'             # Project name
author = 'Lapin Matvey'         # Project author
copyright = '2025, Lapin Matvey'

# --- Sphinx extensions ---
extensions = [
    'myst_parser',              # Markdown support via MyST Parser
    'sphinx_design',            # Design blocks: buttons, cards, grids
    'sphinx.ext.autodoc',       # Auto-generate documentation from Python docstrings
    'sphinx.ext.todo',          # Support for TODO notes
    'sphinx.ext.viewcode',      # Show source code in documentation
    'sphinx.ext.githubpages',   # Create files for publishing on GitHub Pages
    'sphinx.ext.autosectionlabel',  # Allow linking to section titles automatically
]

# Prefix document name to section labels (to avoid duplicate label conflicts)
autosectionlabel_prefix_document = True

# --- MyST Parser settings for Markdown ---
myst_enable_extensions = [
    "colon_fence",              # Enable ::: fenced blocks for warnings, tips, etc.
]

# --- Source file types ---
source_suffix = {
    '.md': 'markdown',          # Treat Markdown files as source
}

# --- Template paths ---
templates_path = [
    (PROJECT_DIR / 'templates').as_posix(),
]

# --- Patterns to exclude from build ---
exclude_patterns = [
    'build',                   # Build output folder
    'Thumbs.db',               # Windows system file
    '.DS_Store',               # macOS system file
    '**/_*.rst'                # Any private RST files
]

# --- Main document ---
master_doc = 'index'          # Entry point for the documentation

# --- HTML output settings ---
html_theme = 'sphinx_rtd_theme'  # Read the Docs theme
html_theme_options = {
    'analytics_id': 'G-EVD5Z6G6NH',  # Google Analytics ID
    'collapse_navigation': False,    # Disable collapsing navigation
    'sticky_navigation': True,       # Keep sidebar fixed
    'navigation_depth': -1,          # Show all levels of headings
}

# --- Static files path (CSS, JS, images) ---
html_static_path = []
