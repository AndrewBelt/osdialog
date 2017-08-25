
CFLAGS = -Wall -g
SOURCES = test.c

ifeq ($(ARCH),lin)
	# Linux
	CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
	LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
	SOURCES += osdialog_gtk2.c
endif

ifeq ($(ARCH),win)
	# Windows
	SOURCES += osdialog_win.c
endif

test: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: test
	./test
