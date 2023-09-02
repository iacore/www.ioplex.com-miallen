SECTION    = 3m
DATE       = "July 22, 2003"
LIB        = "encdec-0.4.0"
TITLE      = "Encdec Library Functions"
COLOR      = "\#000080"
XMLFILES   = xml/encdec.xml
MANFILES   = man/encdec.3m.gz
HTMLFILES  = ref/encdec.html
XSLTCMD    = java -Dcom.jclark.xsl.sax.parser=com.jclark.xml.sax.Driver com.jclark.xsl.sax.Driver
XSLTARG    = section=$(SECTION) date=$(DATE) lib=$(LIB) title=$(TITLE)


all: www/index.html man ref
man: $(MANFILES)
ref: ref/ref.html

man/%.$(SECTION).gz : man/%.$(SECTION)
	gzip -f man/$*.$(SECTION)

man/%.$(SECTION) : xml/%.xml xml/man.xsl
	$(XSLTCMD) $< xml/man.xsl man/$*.$(SECTION) $(XSLTARG)

ref/ref.html: $(XMLFILES) xml/ref.xml xml/ref.xsl
	$(XSLTCMD) xml/ref.xml xml/ref.xsl ref/ref.html $(XSLTARG) color=$(COLOR)

www/index.html: xml/index.xml xml/proj.xsl
	$(XSLTCMD) xml/index.xml xml/proj.xsl www/index.html $(XSLTARG) color=$(COLOR)

clean:
	$(RM) $(MANFILES) $(HTMLFILES) ref/ref.html ref/toc.html ref/overview.html www/index.html

