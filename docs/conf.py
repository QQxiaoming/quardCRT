# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


project = 'quardCRT'
copyright = '2023, Quard(qiaoqiming)'
author = 'Quard(qiaoqiming)'

extensions = ['myst_parser']
templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

language = ['en']
html_theme = 'sphinx_material'
html_title = 'Home'

html_sidebars = {
    "**": ["logo-text.html", "globaltoc.html", "localtoc.html", "searchbox.html"]
}

html_theme_options = {
    'nav_title': 'quardCRT',

    #'color_primary': 'blue',
    #'color_accent': 'light-blue',

    'repo_url': 'https://github.com/QQxiaoming/quardCRT',
    'repo_name': 'quardCRT',
    "logo_icon": "&#xe02f",

    'globaltoc_depth': 1,
    'globaltoc_collapse': True,
    'globaltoc_includehidden': False,

    "nav_links": [
        {"href": "installation","internal": True,"title": "Installation"},
    ],
}
