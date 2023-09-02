SECTION    = 3m
DATE       = "Sep 9, 2004"
LIB        = "domc-0.8.0"
TITLE      = "Document Object Model"
EDGE       = "\#808080"
MAINPANE   = "\#f5f5f5"
LEFTPANE   = "\#f5f5f5"
MIDDLEPANE = "\#f5f5f5"
FOOTER     = "\#808080"
TEXT1      = "\#000000"
TEXT2      = "\#000080"
XSLTCMD    = java -Dcom.jclark.xsl.sax.parser=com.jclark.xml.sax.Driver com.jclark.xsl.sax.Driver
XSLTARG    = section=$(SECTION) date=$(DATE) lib=$(LIB) title=$(TITLE) baseref=$(BASEREF)

XMLFILES   = xml/DOM_CharacterData.xml xml/DOM_Document.xml xml/DOM_Implementation.xml xml/DOM_Element.xml xml/DOM_NamedNodeMap.xml xml/DOM_NodeList.xml xml/DOM_Node.xml xml/DOM_Text.xml
MANFILES   = man/DOM_CharacterData.3m.gz man/DOM_Document.3m.gz man/DOM_Implementation.3m.gz man/DOM_Element.3m.gz man/DOM_NamedNodeMap.3m.gz man/DOM_NodeList.3m.gz man/DOM_Node.3m.gz man/DOM_Text.3m.gz
HTMLFILES  = ref/DOM_CharacterData.html ref/DOM_Document.html ref/DOM_Element.html ref/DOM_Implementation.html ref/DOM_NamedNodeMap.html ref/DOM_Node.html ref/DOM_NodeList.html ref/DOM_Text.html


all: www/index.html man ref/domc.html

man: $(MANFILES)

man/%.$(SECTION).gz : man/%.$(SECTION)
	gzip -f man/$*.$(SECTION)

man/%.$(SECTION) : xml/%.xml xml/man.xsl
	$(XSLTCMD) $< xml/man.xsl man/$*.$(SECTION) $(XSLTARG)

ref/domc.html: $(XMLFILES) xml/domc.xml xml/ref.xsl
	$(XSLTCMD) xml/domc.xml xml/ref.xsl ref/domc.html $(XSLTARG) color=$(TEXT2)

www/index.html: xml/index.xml xml/proj.xsl
	$(XSLTCMD) xml/index.xml xml/proj.xsl www/index.html $(XSLTARG) copyright=$(COPYRIGHT) edge=$(EDGE) mainpane=$(MAINPANE) leftpane=$(LEFTPANE) middlepane=$(MIDDLEPANE) footer=$(FOOTER) text1=$(TEXT1) text2=$(TEXT2)

clean:
	$(RM) $(MANFILES) $(HTMLFILES) ref/domc.html ref/toc.html ref/overview.html www/index.html

