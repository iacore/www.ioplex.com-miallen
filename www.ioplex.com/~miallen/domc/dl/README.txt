DOMC
The Document Object Model in C
http://www.ioplex.com/~miallen/domc/

Thu Sep  9 18:22:32 EDT 2004
domc-0.8.0 released

The  return  value  of DOM_Document_load/fread was not 0 for success and -1
for  error  as  advertised in the documentation. This was a mistake and has
been  corrected.  Even  though  the  change is minor the change in bahavior
warrants  incrementing the minor number vs the update number. A memory leak
has  been  found  and fixed in the expat doctype handler. Additionally some
performance enhancements have been incorporated by the rtfx developers. 

Wed Aug  4 19:20:24 EDT 2004
domc-0.7.1 released

Some  minor  patches  have  been  submitted  regarding  some serialization.
Additionally  the  package has been updated to work with the latest version
of libmba. Actually considering DOMC only needs libmba for the msgno module
the  dependency could be removed alltogether without too much trouble (e.g.
delete all the msgno macros). 

Sat Mar 22 17:23:10 EST 2003

domc-0.7.0 released

Although  the  public  API  does  not reflect it, significant work has been
performed on DOMC internally. Event handling has been largely completed and
additional  work on entities, notations, and DTD oriented functionality has
been added albeit still incomplete. 

--

DOMC  is a light weight C implementation of the DOM as specified in the W3C
Document Object Model Level 1, Level 2, and Level 2 Events recommendations.
The  DOM is a popular API for manipulating XML and HTML documents as a tree
of  nodes  in  memory.  It  is  the  more  sophisticated  but  more  memory
constraining  alternative  to  the  SAX API. This implementation is not W3C
compliant  because  it  lacks  full  support for entity references, DOCTYPE
nodes, DTD default values, and other peripheral functionality. The DOM_Node
type  and  it's associated operations should work well however because what
functionality  is  supported  has  been tested with the DOM Conformace Test
Suite.  A  serialization  module that uses Expat is provided. As of version
0.6.0, DOMC will use the locale dependent multi-byte encoding such as UTF-8
for the DOM_String type. 

DOM Conformance Test Suite Failures

DOMC  is  not  a fully conformant DOM. It does pass 93% of the tests but of
course  it  only takes one failed test to render the implementation useless
for a particular application. See docs/todo for a description of what tests
fail and why. 

INSTALLATION

The  libmba  library  is  required.  Download and install the RPM or tar.gz
from: 

  http://www.ioplex.com/~miallen/libmba/

Now  either  install  the  DOMC RPM or tar.gz. In the later case unpack the
tar.gz  in  an  appropriate  location (e.g. /usr/local/src/) and change the
prefix (default is /usr/local/) in the Makefile if necessary. Now just run:

  # make
  # make install

For  i18n  support  the  encdec  package is required but the encdec package
requires  the __STDC_ISO_10646__ environment which is not supported on many
platforms. The only platform that I am sure of is GNU/Linux with glibc 2.2.
To  enable  i18n support, install encdec, add -DUSE_ENCDEC to the CFLAGS in
the  Makefile, and rebuild the package. In theory it would not be difficult
to  remove  the  encdec  requirement  from  the  expatls.c  module  perhaps
replacing it with ICU or plain iconv support. 

COPYING

Currently all project files are freely distributable under the MIT License.
See each file for details. 

Michael B. Allen <mba2000 ioplex.com>

