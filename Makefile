
CFLAGS = -std=c99 -Wall -g
SOURCES = test.c

ifndef ARCH
$(error ARCH is not defined. Run with `make ARCH=cocoa`, win32, or gtk2)
endif

SOURCES += osdialog.c

ifeq ($(ARCH),gtk2)
	CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
	LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
	SOURCES += osdialog_gtk2.c
endif

ifeq ($(ARCH),win32)
	LDFLAGS += -lcomdlg32
	SOURCES += osdialog_win32.c
endif

ifeq ($(ARCH),cocoa)
	LDFLAGS += -framework AppKit
	SOURCES += osdialog_cocoa.m
	CFLAGS += -mmacosx-version-min=10.7
endif

test: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: test
	./test

clean:
	rm -rfv test
