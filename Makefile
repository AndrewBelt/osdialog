
CFLAGS = -Wall -g
SOURCES = test.c

ifndef ARCH
$(error ARCH is not defined. Run with `make ARCH=mac`, win, or lin)
endif

ifeq ($(ARCH),lin)
	# Linux
	CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
	LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
	SOURCES += osdialog_gtk2.c
endif

ifeq ($(ARCH),win)
	# Windows
	LDFLAGS += -lcomdlg32
	SOURCES += osdialog_win.c
endif

ifeq ($(ARCH),mac)
	# MacOS
	LDFLAGS += -framework AppKit
	SOURCES += osdialog_mac.m
endif

test: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: test
	./test

clean:
	rm -rfv test