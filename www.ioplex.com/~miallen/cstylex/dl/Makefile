SECTION    = 3m
DATE       = "May 8, 2004"
LIB        = "cstylex-0.2.0"
DISTRO     = $(LIB)
TITLE      = "My Library Functions"
COPYRIGHT  = "Michael B Allen <mba2000 ioplex.com>"
BASEREF    = "http://www.mylib.org/dl/docs/"
EDGE       = "\#e0e0e0"
MAINPANE   = "\#f0f0f0"
LEFTPANE   = "\#f0f0f0"
MIDDLEPANE = "\#f0f0f0"
FOOTER     = "\#e0e0e0"
TEXT1      = "\#000000"
TEXT2      = "\#404040"
XSLTCMD    = java -Dcom.jclark.xsl.sax.parser=com.jclark.xml.sax.Driver com.jclark.xsl.sax.Driver
XSLTARG    = section=$(SECTION) date=$(DATE) lib=$(LIB) title=$(TITLE) baseref=$(BASEREF)

XMLFILES   = xml/foo.xml
MANFILES   = man/foo.3m.gz
HTMLFILES  = ref/foo.html

all: www/index.html man ref/mylib.html

man: $(MANFILES)

man/%.$(SECTION).gz : man/%.$(SECTION)
	gzip -f man/$*.$(SECTION)

man/%.$(SECTION) : xml/%.xml xml/man.xsl
	$(XSLTCMD) $< xml/man.xsl man/$*.$(SECTION) $(XSLTARG)

ref/mylib.html: $(XMLFILES) xml/mylib.xml xml/ref.xsl
	$(XSLTCMD) xml/mylib.xml xml/ref.xsl ref/mylib.html $(XSLTARG) color="#000080"

www/index.html: xml/index.xml xml/proj.xsl Makefile
	$(XSLTCMD) xml/index.xml xml/proj.xsl www/index.html $(XSLTARG) copyright=$(COPYRIGHT) edge=$(EDGE) mainpane=$(MAINPANE) leftpane=$(LEFTPANE) middlepane=$(MIDDLEPANE) footer=$(FOOTER) text1=$(TEXT1) text2=$(TEXT2)

zip:
	cd .. && zip -lr $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/man/*
	mv .$(DISTRO).zip $(DISTRO).zip

clean:
	$(RM) $(MANFILES) $(HTMLFILES) ref/mylib.html ref/toc.html ref/overview.html www/index.html

