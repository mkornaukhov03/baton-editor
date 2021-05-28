# Baton-editor

Baton is a code editor for C++ programming language \
Design document: https://bit.ly/3alkddH

## Features:

* Source code highlighting
* C++ auto-completion: dynamic completion options displaying and quick insertion
* Document navigation: current line and column display
* Split option: allows developer to work with several files simultaneously
* Editor features: adjustable text size, auto indentation and line-numbering, auto parentheses highlighting and
  completion
* Command window: make possible to invoke commands within Baton-editor
* Diagnostic report window: provides diagnostic information to find and fix bugs and misspellings
* Auto highlighting adjustment: syntax highlighting adjusts to system theme coloring
* User-friendly builtin directory tree: allows developer to view current directory and switch files with few clicks,
  root directory can also be changed in the menu
* Hotkeys for basic operations

## Requirements:
* Qt 5.15
* Clangd
* Ubuntu 20.10 or later
* CMake 3.10 or later
* GCC 9.3.0 or later

## Building instructions

1. Go to the build folder:
    ```bash
    cd build
    ```
2. Configure:
    ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ../..
    ```
3. Build:
   ```bash
   make -j4
   ```
4. Execute:
   ```bash
   ./baton
   ```
   
## Examples
![alt text](https://github.com/mkornaukhov03/baton-editor/blob/text-editor/images/example.gif "Demonstration")

![alt text](https://github.com/mkornaukhov03/baton-editor/blob/text-editor/images/example-image.jpg "Screenshot")