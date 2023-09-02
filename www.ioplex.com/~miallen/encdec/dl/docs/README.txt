CStyleX
XSLT Genereated Documentation for C Libraries
htt://www.eskimo.com/~miallen/cstylex/

The CStyleX package provides ref.xsl, man.xsl, and proj.xsl XSLT transforms
that will convert an XML input file describing a library of C routines into
an HTML Reference, man pages, and project webpage. Examples include: 

  http://www.eskimo.com/~miallen/domc/
  http://www.eskimo.com/~miallen/libmba/
  http://www.eskimo.com/~miallen/encdec/

CStyleX  does  not  attempt  to  parse C source or headers. The author must
create   an  XML  input  file  that  conforms  to  a  certain  syntax  (see
xml/SYNTAX.txt)  and  run  'make'  to  output  the  documentation.  An XSLT
processor   is   required.   The   Makefile   is   configured   to  use  XT
(http://www.blnz.com/xt/). The exact instructions are as follows: 

o  Unpack  CStyle-0.1.0.tar.gz  in  your project directory and rename it to
'docs' (or something appropriate). This directory contains the following: 

  Makefile   - Makefile  to build the HTML reference, man pages, or project
               web page
  man        - Directory in which generated man pages will be written
  README.txt - This file
  ref        - Directory in which generated HTML reference will be written
  www        - Directory in which the project web page will be generated
  xml        - Input   source  XML,  "blank.xml"  to  start  new  interface
               descriptions,   and  man.xsl,  ref.xsl,  and  proj.xsl  XSLT
               transforms 
  xml/SYNTAX.txt - Hierarchy  of  tags  to  aid  in  creating a valid input
               files.  See  other  input  files  from example projects like
               http://www.eskimo.com/~miallen/libmba/dl/docs/xml/cfg.xml 

o  In the xml directory, cp blank.xml to a file appropriately named for the
interface  of  your  library.  Change  and  add  to  this "blank" interface
definition.  Use  SYNTAX.txt  and  other  examples  (libmba,  domc,  encdec
libraries) as a guide.

o  Change  the  entity  references in xml/ref.xml to include your interface
definitions. If you have only one interface in the reference you can simply
choose  to  ignore  the framed index.html. If you have many interfaces from
different  packages  they  can  be  included in ref.xml to make one big API
reference. 

o  Edit the Makefile to reference the input files and make the docs. If you
get "character not allowed" errors check your entity references. Characters
like  '>'  or  '&'  need  to  be escaped in XML with entity references like
'&gt;' and '&amp;'. 
