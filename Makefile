LDFLAGS=-lm
PREFIX=/usr/local
MANPREFIX=$(PREFIX)/man
INSTALL=install -D

all: rad

install: rad
	$(INSTALL) rad $(DESTDIR)$(PREFIX)/bin/rad
	$(INSTALL) rad.1 $(DESTDIR)$(MANPREFIX)/man1/rad.1

.PHONE: clean
clean:
	$(RM) rad
