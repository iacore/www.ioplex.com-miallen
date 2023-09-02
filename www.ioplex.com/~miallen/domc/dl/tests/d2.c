#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <domc.h>

int
main(int argc, char *argv[])
{
    DOM_Document *doc;
    DOM_Element *root, *foo, *bar, *clone, *zap;
	DOM_Text *tex;
	DOM_NodeList *foos;
	unsigned long i;

	if (!setlocale(LC_CTYPE, "")) {
			fprintf(stderr, "Can't set the specified locale! "
							"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}

    doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
    root = DOM_Document_createElement(doc, "root");
    doc->u.Document.documentElement = root;
    foo = DOM_Document_createElement(doc, "foo");
    bar = DOM_Document_createElement(doc, "bar");
	tex = DOM_Document_createTextNode(doc, "This is a text node.");
    zap = DOM_Document_createElement(doc, "zap");

	DOM_Node_appendChild(root, foo);
	DOM_Node_appendChild(root, bar);
	DOM_Node_appendChild(bar, tex);
	DOM_Node_insertBefore(bar, zap, DOM_Text_splitText(tex, 6));
	DOM_Text_splitText(tex, 3);

	clone = DOM_Node_cloneNode(root, 1);
	DOM_Node_insertBefore(bar, clone, tex);

	foos = DOM_Document_getElementsByTagName(doc, "#text");
	for (i = 0; i < foos->length; i++) {
		printf("foo element: %s\n", DOM_NodeList_item(foos, i)->nodeValue);
	}
	DOM_Document_destroyNodeList(doc, foos, 0);

	DOM_Element_normalize(clone);

    DOM_DocumentLS_fwrite(doc, stdout);
    DOM_Document_destroyNode(doc, doc);

    return EXIT_SUCCESS;
}
