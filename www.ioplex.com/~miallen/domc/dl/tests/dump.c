#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <mba/msgno.h>
#include <domc.h>

int
main(int argc, char *argv[])
{
	DOM_Document *doc;

	if (argc < 2) {
		MSG("Must specify XML file");
		return EXIT_FAILURE;
	}

	if (!setlocale(LC_CTYPE, "")) {
		MSG("Cannot set locale");
		return EXIT_FAILURE;
	}

	DOM_Exception = 0;
	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, argv[1]) == -1 ||
			DOM_DocumentLS_fwrite(doc, stdout) == -1) {
		MSG("Failed to process %s", argv[1]);
		return EXIT_FAILURE;
	}

	DOM_Document_destroyNode(doc, doc);

	return EXIT_SUCCESS;
}
