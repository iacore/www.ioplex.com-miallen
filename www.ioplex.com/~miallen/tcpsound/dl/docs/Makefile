SECTION    = 1
DATE       = "Sep 5, 2005"
PACKAGE    = "tcpsound-0.3.1"
TITLE      = ""
COPYRIGHT  = "Michael B Allen <mba2000 ioplex.com>"
BASEREF    = "http://www.ioplex.com/~miallen/tcpsound/dl/docs/"
EDGE       = "\#e0e0e0"
MAINPANE   = "\#f0f0f0"
LEFTPANE   = "\#f0f0f0"
MIDDLEPANE = "\#f0f0f0"
FOOTER     = "\#e0e0e0"
TEXT1      = "\#000000"
TEXT2      = "\#404040"
XSLTCMD    = java -classpath xt.jar:xp.jar:. -Dcom.jclark.xsl.sax.parser=com.jclark.xml.sax.Driver com.jclark.xsl.sax.Driver
XSLTARG    = section=$(SECTION) date=$(DATE) lib=$(PACKAGE) title=$(TITLE) baseref=$(BASEREF)

XMLFILES   = xml/tcpsound.xml
MANFILES   = man/tcpsound.1.gz

all: www/index.html $(MANFILES)

www/index.html: xml/index.xml xml/proj.xsl Makefile
	$(XSLTCMD) xml/index.xml xml/proj.xsl www/index.html $(XSLTARG) copyright=$(COPYRIGHT) edge=$(EDGE) mainpane=$(MAINPANE) leftpane=$(LEFTPANE) middlepane=$(MIDDLEPANE) footer=$(FOOTER) text1=$(TEXT1) text2=$(TEXT2)

www/%.html: $(XMLFILES) xml/ref.xsl
	$(XSLTCMD) $< xml/ref.xsl www/$*.html $(XSLTARG) color="#000080"

man/%.$(SECTION) : xml/%.xml xml/man.xsl
	$(XSLTCMD) $< xml/man.xsl man/$*.$(SECTION) $(XSLTARG)

man/%.$(SECTION).gz : man/%.$(SECTION)
	gzip -f man/$*.$(SECTION)


clean:
	$(RM) www/index.html $(MANFILES)

