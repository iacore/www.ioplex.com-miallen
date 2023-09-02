#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <domc.h>

int
main(int argc, char *argv[])
{
	DOM_Document *doc;
	DOM_DocumentFragment *dfrag;
	DOM_Element *root, *e0, *e1;
	DOM_Text *t0;
	DOM_Node *node;

	if (argc < 2) {
		fprintf(stderr, "Must specify XML file\n");
		return EXIT_FAILURE;
	}

	if (!setlocale(LC_CTYPE, "")) {
			fprintf(stderr, "Can't set the specified locale! "
							"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}

	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, argv[1]) == -1) {
		fprintf(stderr, "Error trying to load XML file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	root = doc->u.Document.documentElement;
	for (node = root->firstChild;
				node && node->nodeType != DOM_ELEMENT_NODE;
				node = node->nextSibling) {
		;
	}
	if (node == NULL) {
		fprintf(stderr, "The document has no element children\n");
		return EXIT_FAILURE;
	}

	dfrag = DOM_Document_createDocumentFragment(doc);
	e0 = DOM_Document_createElement(doc, "foo");
	e1 = DOM_Document_createElement(doc, "bar");
	t0 = DOM_Document_createTextNode(doc, "This tests the DocumentFragment operations such as properly moving nodes from the DocumentFragment into the children of another Node for the DOM_Node_appendChild, DOM_Node_insertBefore, etc.");

	DOM_Node_appendChild(dfrag, e0);
	DOM_Node_appendChild(dfrag, e1);
	DOM_Node_appendChild(dfrag, t0);

	if (DOM_Node_appendChild(node, dfrag) == NULL) {
		fprintf(stderr, "Error aappending child node\n");
	}
	if (DOM_DocumentLS_fwrite(doc, stdout) == -1) {
		fprintf(stderr, "Error while trying to save the DOM tree to stdout\n");
		return EXIT_FAILURE;
	}

	DOM_Document_destroyNode(doc, dfrag);
	DOM_Document_destroyNode(doc, doc);

	return EXIT_SUCCESS;
}
