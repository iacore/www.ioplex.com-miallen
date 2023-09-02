#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <domc.h>

int
d5(const char *filename)
{
	DOM_Document *doc, *clone;

	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, filename) == 0) {
		return 0;
	}

	clone = DOM_Node_cloneNode(doc->u.Document.documentElement, 1);

	if (DOM_DocumentLS_fwrite(doc, stdout) == 0 ||
		DOM_DocumentLS_save(clone, "clone.xml", NULL) == 0) {
		return 0;
	}
	DOM_Document_destroyNode(doc, doc);
	DOM_Document_destroyNode(clone, clone);
	return 1;
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Must provide XML filename\n");
		return EXIT_FAILURE;
	}
	if (!setlocale(LC_CTYPE, "")) {
			fprintf(stderr, "Can't set the specified locale! "
							"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}
	DOM_Exception = 0;
	if (!d5(argv[1])) {
		MNO(DOM_Exception);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
