
CC ?= gcc
AR ?= ar
CFLAGS ?= -g -O2

prefix:=/usr
bindir=$(prefix)/bin
libdir=$(prefix)/lib
incdir=$(prefix)/include
docdir=$(prefix)/share/doc/Msys
DLLVER=0
EXE=.exe

STATICLIB=libcrypt.a
SHAREDLIB=msys-crypt-$(DLLVER).dll
IMPORTLIB=libcrypt.dll.a

APPS=crypt$(EXE)
LIBS=$(STATICLIB) $(SHAREDLIB)

all: $(APPS) $(LIBS)

crypt$(EXE): crypt.o $(LIBS)
	$(CC) -o $@ crypt.o -L. -lcrypt

$(STATICLIB): encrypt.o
	$(AR) rv $@ encrypt.o

$(SHAREDLIB): encrypt.o
	$(CC) -shared -Wl,--gc-sections -Wl,--out-implib=$(IMPORTLIB) crypt.def \
	$^ -o $@

encrypt.o: encrypt.h

distclean: clean

clean:
	-rm *.o *.exe *.a *.dll

install: all
	install -d $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(libdir)
	install -d $(DESTDIR)$(incdir)
	install -d $(DESTDIR)$(docdir)
	install -m 755 $(APPS)         $(DESTDIR)$(bindir)
	install -m 644 encrypt.h       $(DESTDIR)$(incdir)/crypt.h
	install -m 644 $(STATICLIB)    $(DESTDIR)$(libdir)
	install -m 644 $(IMPORTLIB)    $(DESTDIR)$(libdir)
	install -m 755 $(SHAREDLIB)    $(DESTDIR)$(bindir)
	install -m 644 crypt.README    $(DESTDIR)$(docdir)

