
CFLAGS = -Wall -g
SOURCES = test.c

# Linux
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
SOURCES += osdialog_gtk2.c


test: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: test
	./test
