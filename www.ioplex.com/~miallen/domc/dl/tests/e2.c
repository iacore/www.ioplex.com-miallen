#include <stdlib.h>
#include <stdio.h>
#define HAVE_VARMACRO 1
#include <mba/msgno.h>
#include <domc.h>

void
event0_fn(DOM_EventListener *listener, DOM_Event *evt)
{
	fprintf(stderr, "event0_fn Fired! -- type=%s,target=%s,currentTarget=%s,eventPhase=%d,bubbles=%d\n", evt->type, evt->target->nodeName, evt->currentTarget->nodeName, evt->eventPhase, evt->bubbles);
	listener = NULL;
}
void
event1_fn(DOM_EventListener *listener, DOM_Event *evt)
{
	fprintf(stderr, "event1_fn Fired! -- type=%s,target=%s,currentTarget=%s,eventPhase=%d,bubbles=%d\n", evt->type, evt->target->nodeName, evt->currentTarget->nodeName, evt->eventPhase, evt->bubbles);
	listener = NULL;
}

int
main(int argc, char *argv[])
{
	DOM_Document *doc;
	DOM_Element *elem;
	DOM_Event *evt;
	DOM_NodeList *list;

	if (argc < 2) {
		fprintf(stderr, "Must provide XML filename\n");
		return EXIT_FAILURE;
	}

	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, argv[1]) == -1) {
		if (DOM_Exception) {
			MNO(DOM_Exception);
		}
		return EXIT_FAILURE;
	}

	list = DOM_Document_getElementsByTagName(doc, "three");
	elem = DOM_NodeList_item(list, 0);
	DOM_EventTarget_addEventListener(elem->parentNode, "Events", &event0_fn, event0_fn, 0);
	DOM_EventTarget_addEventListener(elem->parentNode, "Events", &event1_fn, event1_fn, 0);
	DOM_EventTarget_removeEventListener(elem->parentNode, "Events", &event0_fn, event0_fn, 0);
	DOM_EventTarget_removeEventListener(elem->parentNode, "Events", &event1_fn, event1_fn, 0);
	DOM_EventTarget_addEventListener(elem->parentNode, "Events", &event0_fn, event0_fn, 1);

	fprintf(stderr, "target[%s]\n", elem->parentNode->nodeName);

	evt = DOM_DocumentEvent_createEvent(doc, "Events");
	DOM_Event_initEvent(evt, "Events", 1, 1);
	if (DOM_EventTarget_dispatchEvent(elem, evt)) {
		fprintf(stderr, "default prevented\n");
	}

	DOM_DocumentEvent_destroyEvent(doc, evt);
	DOM_Document_destroyNodeList(doc, list, 0);
	DOM_Document_destroyNode(doc, doc);

	return EXIT_SUCCESS;
}
