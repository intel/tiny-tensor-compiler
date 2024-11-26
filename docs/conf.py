# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

project = 'Tiny Tensor Compiler'
copyright = '2024, Intel Corporation'
author = 'Intel'

extensions = ['breathe', 'sphinx.ext.mathjax', 'sphinx_tabs.tabs']

templates_path = ['_templates']
exclude_patterns = []

html_theme = 'sphinx_book_theme'
html_static_path = ['_static']
html_theme_options = {
    'repository_provider': 'github',
    'repository_url': 'https://github.com/intel/tiny-tensor-compiler',
    'use_repository_button': True,
    'use_issues_button': True,
    'use_source_button': True,
    'use_edit_page_button': True,
    'path_to_docs': 'docs',
    'navigation_with_keys': False
}
html_css_files = ['fix-scrollbar-bug.css']

breathe_default_project = 'api'
breathe_show_include = False
breathe_show_define_initializer = True
breathe_show_enumvalue_initializer = True
breathe_default_members = ('members',)
breathe_domain_by_extension = {
    'h': 'c',
    'hpp': 'cpp',
    'cpp': 'cpp'
}

toc_object_entries = False
