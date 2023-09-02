LIBICONV   = ../libiconv-1.7
prefix     = /usr/local
includedir = $(prefix)/include
libdir     = $(prefix)/lib
mandir     = $(prefix)/man
CC         = gcc
LIBNAME    = encdec
MAJVERSION = 0.4
MINVERSION = 0.4.0
ARNAME     = lib$(LIBNAME).a
SONAME     = lib$(LIBNAME).so.$(MINVERSION)
SOVERSION  = lib$(LIBNAME).so.$(MAJVERSION)
DISTRO     = $(LIBNAME)-$(MINVERSION)
RPM_OPT_FLAGS = -O2
CFLAGS     = -Wall $(RPM_OPT_FLAGS) -I$(includedir) -L$(libdir)
#CFLAGS     = -Wall -W -I$(includedir) -L$(libdir) $(RPM_OPT_FLAGS) -ansi -pedantic -Wbad-function-cast -Wcast-align -Wcast-qual -Wchar-subscripts -Winline -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wtraditional -Wconversion -Waggregate-return -Wno-parentheses
OBJS       = src/encdec.o src/encdec_iconv.o

all: $(ARNAME) $(SONAME)

$(SONAME): $(OBJS)
	$(CC) -shared $(OBJS) $(LIBICONV)/libcharset/lib/localcharset.lo -Wl,-h,$(SOVERSION) -o $(SONAME)

$(ARNAME):
	$(CC) $(CFLAGS) -c -o src/encdec.o src/encdec.c
	$(CC) $(CFLAGS) -I$(LIBICONV)/include -I$(LIBICONV)/lib -I$(LIBICONV)/libcharset/include -L$(LIBICONV)/lib -c -o src/encdec_iconv.o src/encdec_iconv.c
	ar rv $(ARNAME) $(OBJS) $(LIBICONV)/libcharset/lib/localcharset.o
	$(RM) $(OBJS)

noiconv: src/encdec.o
	$(CC) -shared src/encdec.o -Wl,-h,$(SOVERSION) -o $(SONAME)

java: src/Encdec.java
	javac src/Encdec.java

src/encdec.o: src/encdec.h src/encdec.c
	$(CC) $(CFLAGS) -fpic -c -o src/encdec.o src/encdec.c

src/encdec_iconv.o: src/encdec.h src/encdec_iconv.c
	$(CC) $(CFLAGS) -I$(LIBICONV)/include -I$(LIBICONV)/lib -I$(LIBICONV)/libcharset/include -L$(LIBICONV)/lib -fpic -c -o src/encdec_iconv.o src/encdec_iconv.c

.c.o:
	$(CC) $(CFLAGS) -fpic -c -o src/$*.o src/$<


install: $(SONAME)
	install -d $(libdir)
	install -d $(includedir)
	install -d $(mandir)/man3
	install -m 644 $(ARNAME) $(libdir)
	install -m 755 $(SONAME) $(libdir)/$(SONAME)
	cd $(libdir) && ln -sf $(SONAME) $(SOVERSION) && ln -sf $(SONAME) lib$(LIBNAME).so
	install -m 444 src/encdec.h $(includedir)/encdec.h
	-install -m 444 docs/man/encdec.3m.gz $(mandir)/man3/encdec.3m.gz
	-ldconfig $(libdir)

zip:
	-#cd .. && zip -lr $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/docs/man/* $(DISTRO)/tests/data* $(DISTRO)/tests/u*.html $(DISTRO)/docs/*.gz
	-rm docs/www/dl
	cd .. && zip -lr $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/docs/man/* $(DISTRO)/tests/data* $(DISTRO)/tests/u*.html $(DISTRO)/docs/*.gz $(DISTRO)/encdec.lib $(DISTRO)/encdec.dll $(DISTRO)/encdec_s.lib
	cd .. && zip -ur $(DISTRO)/.$(DISTRO).zip $(DISTRO)
	mv .$(DISTRO).zip $(DISTRO).zip

clean:
	rm -f src/*.o src/*.class $(ARNAME) $(SONAME) $(includedir)/encdec.h $(libdir)/$(ARNAME) $(libdir)/$(SONAME) $(libdir)/$(SOVERSION) $(libdir)/lib$(LIBNAME).so $(mandir)/man3/encdec.3.gz $(DISTRO).zip

