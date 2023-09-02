#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#define HAVE_VARMACRO 1
#include <mba/msgno.h>
#include <domc.h>

void
event_fn(DOM_EventListener *listener, DOM_Event *evt)
{
	fprintf(stderr, "type=%s,target=%s,currentTarget=%s,eventPhase=%d,bubbles=%d\n", evt->type, evt->target->nodeName, evt->currentTarget->nodeName, evt->eventPhase, evt->bubbles);
	listener = NULL;
}

int
main(int argc, char *argv[])
{
	DOM_Document *doc;
	DOM_Element *root;
	DOM_Event *evt;

	if (argc < 2) {
		MSG("Must provide XML filename");
		return EXIT_FAILURE;
	}

	if (!setlocale(LC_CTYPE, "")) {
			fprintf(stderr, "Can't set the specified locale! "
				"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}

	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, argv[1]) == -1) {
		if (DOM_Exception) {
			MSG("Event test failed");
		}
		return EXIT_FAILURE;
	}

	root = doc->u.Document.documentElement;
	DOM_EventTarget_addEventListener(root, "Events", &event_fn, event_fn, 1);

	MSG("target[%s]", root->nodeName);

	evt = DOM_DocumentEvent_createEvent(doc, "Events");
	DOM_Event_initEvent(evt, "Events", 1, 1);
	DOM_EventTarget_dispatchEvent(root->firstChild, evt);

	DOM_DocumentEvent_destroyEvent(doc, evt);
	DOM_Document_destroyNode(doc, doc);

	return EXIT_SUCCESS;
}
