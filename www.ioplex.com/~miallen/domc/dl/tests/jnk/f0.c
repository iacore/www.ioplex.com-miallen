#include <stdlib.h>
#include <stdio.h>
#include <domc.h>

int
main(int argc, char *argv[])
{
	DOM_Document *doc;

	if (argc < 2) {
		fprintf(stderr, "must specify XML file\n");
		return EXIT_FAILURE;
	}

	DOM_Exception = 0;
	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, argv[1]) == 0 ||
			DOM_DocumentLS_fwrite(doc, stdout) == 0) {
		MNO(DOM_Exception);
		return EXIT_FAILURE;
	}

	DOM_Document_destroyNode(doc, doc);

	return EXIT_SUCCESS;
}
