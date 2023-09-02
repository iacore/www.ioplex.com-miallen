prefix     = /usr/local
includedir = $(prefix)/include
libdir     = $(prefix)/lib
mandir     = $(prefix)/man
CC         = gcc
LIBNAME    = domc
MAJVERSION = 0.8
MINVERSION = 0.8.0
ARNAME     = lib$(LIBNAME).a
SONAME     = lib$(LIBNAME).so.$(MINVERSION)
SOVERSION  = lib$(LIBNAME).so.$(MAJVERSION)
DISTRO     = $(LIBNAME)-$(MINVERSION)
RPM_OPT_FLAGS = -O2
CFLAGS     = -Wall -W -g -DMSGNO $(RPM_OPT_FLAGS) -I$(includedir) -L$(libdir)
#CFLAGS     = -Wall -W -DMSGNO -I$(includedir) -L$(libdir) $(RPM_OPT_FLAGS) -ansi -pedantic -Wbad-function-cast -Wcast-align -Wcast-qual -Wchar-subscripts -Winline -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings -Wtraditional -Wconversion -Waggregate-return -Wno-parentheses
OBJS       = src/expatls.o src/events.o src/node.o src/nodelist.o src/namednodemap.o src/dom.o src/timestamp.o src/wcwidth.o src/mbs.o
MAN        = DOM_CharacterData.3m.gz DOM_Document.3m.gz DOM_Element.3m.gz DOM_Implementation.3m.gz DOM_NamedNodeMap.3m.gz DOM_Node.3m.gz DOM_NodeList.3m.gz DOM_Text.3m.gz

all: $(ARNAME)($(OBJS)) $(SONAME) src/defines.h

$(SONAME): $(OBJS)
	$(CC) -shared $(OBJS) -L$(libdir) -lmba -lexpat -Wl,-h,$(SOVERSION) -o $(SONAME)

.c.a:
	$(CC) $(CFLAGS) -c -o $*.o $<
	ar rv $@ $*.o
	rm $*.o

.c.o:
	$(CC) $(CFLAGS) -fpic -c -o $*.o $<

install: $(SONAME)
	install -d $(libdir)
	install -d $(includedir)
	install -d $(mandir)/man3
	install -m 644 $(ARNAME) $(libdir)
	install -m 755 $(SONAME) $(libdir)
	cd $(libdir) && ln -sf $(SONAME) $(SOVERSION) && ln -sf $(SONAME) lib$(LIBNAME).so
	install -m 444 src/domc.h $(includedir)
	-install -m 444 docs/man/*.3m.gz $(mandir)/man3
	-/sbin/ldconfig $(libdir)

zip:
	cd .. && zip -lr $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/docs/man/* $(DISTRO)/tests/utf8* $(DISTRO)/domc.lib $(DISTRO)/domc.dll $(DISTRO)/domc_s.lib
	cd .. && zip -ur $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/docs/man/*
	mv .$(DISTRO).zip $(DISTRO).zip

clean:
	rm -f $(OBJS) $(ARNAME) $(SONAME) $(includedir)/domc.h $(libdir)/$(ARNAME) $(libdir)/$(SONAME) $(libdir)/$(SOVERSION) $(libdir)/lib$(LIBNAME).so $(DISTRO).zip
	cd $(mandir)/man3 && rm -f $(MAN)

