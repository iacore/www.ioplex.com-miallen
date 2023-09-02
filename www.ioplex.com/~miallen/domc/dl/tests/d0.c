#include <stdio.h>
#include <locale.h>
#include <mba/msgno.h>
#include <domc.h>

int
main(void)
{
    DOM_Document *doc;
    DOM_Element *root, *foo, *bar, *clo, *zig, *six;
	DOM_Text *tex;
	unsigned long i, n;
	DOM_Attr *a;

	if (!setlocale(LC_CTYPE, "")) {
			fprintf(stderr, "Can't set the specified locale! "
							"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}

    doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
    root = DOM_Document_createElement(doc, "root");
    foo = DOM_Document_createElement(doc, "foo");
    bar = DOM_Document_createElement(doc, "bar");
    zig = DOM_Document_createElement(doc, "zig");
    six = DOM_Document_createElement(doc, "six");
	tex = DOM_Document_createTextNode(doc, "This is text.");

	DOM_CharacterData_appendData(tex, "More text.");
	DOM_CharacterData_insertData(tex, 5, "Michael.");

	DOM_Node_appendChild(doc, root);
	DOM_Node_appendChild(root, foo);
	DOM_Node_appendChild(root, bar);
	DOM_Node_appendChild(bar, tex);

	clo = DOM_Node_cloneNode(bar, 1);
	DOM_CharacterData_replaceData(tex, 5, 8, "HELLO, WORLD");
	DOM_CharacterData_deleteData(tex, 2, 2);
	DOM_Text_splitText(tex, 14);
	DOM_DocumentLS_fwrite(root, stdout);

	DOM_Node_insertBefore(foo, clo, NULL);
	DOM_Node_replaceChild(bar, zig, tex);
	DOM_Node_removeChild(root, bar);

	DOM_Node_insertBefore(clo, tex, NULL);
	DOM_Node_insertBefore(clo, six, tex);
	DOM_Node_insertBefore(clo, bar, tex);

	n = clo->childNodes->length;
	for (i = 0; i < n; i++) {
		printf("%s\n", DOM_NodeList_item(clo->childNodes, i)->nodeName);
	}

	a = DOM_Document_createAttribute(doc, "att0");
	DOM_Document_destroyNode(doc, a);

	DOM_DocumentLS_fwrite(NULL, stdout);
    DOM_Document_destroyNode(doc, doc);

    return EXIT_SUCCESS;
}
