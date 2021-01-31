# `osdialog`

A cross platform wrapper for OS dialogs like file save, open, message boxes, inputs, color picking, etc.

Currently supports macOS, Windows, and GTK2/GTK3 on Linux.

`osdialog` is released into the public domain [(CC0)](LICENSE.txt).
If you decide to use osdialog in your project, [please let me know](https://github.com/AndrewBelt/osdialog/issues/9).

## Using

There is only one header: simply add `inc/osdialog.h` to your include directory.

For the sources, you have two options:

-   just add `src/osdialog.c` and the appropriate `src/osdialog_<toolkit>.(c|m)` file to your application's sources
-   ***or,*** produce a library using `cmake`:
    ```sh
    cmake -S . -B build/
        # [-DOSDLG_TOOLKIT=<GTK|GTK3|ZENITY>]
        # [-DOSDLG_BUILD_TESTS=<ON|OFF>]
        # [-DBUILD_SHARED_LIBS=<ON|OFF>]
    cmake --build build/
    ```
