# The Makefile is only for building the osdialog test binary.
# You don't need to use it for your application, but it might be helpful for suggesting compiler flags.

CFLAGS = -g -Wall -Wextra -std=c99 -pedantic
SOURCES = test.c

ifndef ARCH
$(error ARCH is not defined. Run with `make ARCH=mac`, win, gtk2, gtk3, or zenity)
endif

SOURCES += osdialog.c

ifeq ($(ARCH),gtk2)
	CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
	LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
	SOURCES += osdialog_gtk2.c
endif

ifeq ($(ARCH),gtk3)
	CFLAGS += $(shell pkg-config --cflags gtk+-3.0)
	LDFLAGS += $(shell pkg-config --libs gtk+-3.0)
	SOURCES += osdialog_gtk3.c
endif

ifeq ($(ARCH),win)
	LDFLAGS += -lcomdlg32
	SOURCES += osdialog_win.c
endif

ifeq ($(ARCH),mac)
	LDFLAGS += -framework AppKit
	SOURCES += osdialog_mac.m
	CFLAGS += -mmacosx-version-min=10.7
endif

ifeq ($(ARCH),zenity)
	SOURCES += osdialog_zenity.c
endif

test: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: test
	./$^

clean:
	rm -rfv test
