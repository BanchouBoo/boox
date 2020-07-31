CC ?= cc

SRC = src/boox.c src/xcb/connection.c src/xcb/selection.c src/parsing/parsing.c

CFLAGS = -Wall -Wextra -pedantic
DEPENDS = xcb xcb-cursor xcb-shape
LDFLAGS = `pkg-config --cflags --libs $(DEPENDS)`

PREFIX ?= /usr/local
TARGET ?= boox

all:
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

install: all
	install -D -m 755 $(TARGET) "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"
