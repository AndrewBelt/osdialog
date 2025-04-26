# 🗨️ osdialog

A cross platform wrapper for OS dialogs like file save, open, message boxes, inputs, color picking, etc.

osdialog currently supports macOS, Windows, and GTK 2/3 and [Zenity](https://gitlab.gnome.org/GNOME/zenity) on Linux.

This software is released into the public domain ([CC0](LICENSE.txt)).
If you decide to use osdialog in your project, feel free to [let me know](https://github.com/AndrewBelt/osdialog/issues/9).

## Usage

Include `osdialog.h` in your program, and add `osdialog.c` and the appropriate `osdialog_*.c/.m` file to your program's sources.
See [osdialog.h](osdialog.h) for documentation.

The [Makefile](Makefile) is only for building the osdialog test binary.
You don't need to use it, but it might be helpful for suggesting compiler flags.
