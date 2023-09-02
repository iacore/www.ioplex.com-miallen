#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <mba/msgno.h>
#include <mba/linkedlist.h>
#include <domc.h>

void nodeRemoved(void *this, DOM_Event *evt);
void
nodeInserted(void *this, DOM_Event *evt)
{
	DOM_Node *copy = this, *child;

	if (evt->eventPhase != DOM_EVENT_CAPTURING_PHASE ||
			evt->target->parentNode != evt->relatedNode) {
		return;
	}

	switch (evt->target->nodeType) {
		case DOM_ELEMENT_NODE:
			child = DOM_Document_createElement(copy->ownerDocument, evt->target->nodeName);
			break;
		case DOM_TEXT_NODE:
			child = DOM_Document_createTextNode(copy->ownerDocument, evt->target->nodeValue);
			break;
		case DOM_COMMENT_NODE:
			child = DOM_Document_createComment(copy->ownerDocument, evt->target->nodeValue);
			break;
		default:
			MSG("Don't know how to copy this type: %d", evt->target->nodeType);
			return;
	}
	if (DOM_Node_appendChild(copy, child) == NULL) {
		MSG("Append failed");
		return;
	}
	DOM_EventTarget_addEventListener(evt->target, "DOMNodeInserted", child, nodeInserted, 1);
	DOM_EventTarget_addEventListener(evt->target, "DOMNodeRemoved", child, nodeRemoved, 1);
}
void
nodeRemoved(void *this, DOM_Event *evt)
{
	DOM_Node *copy = this;

	if (evt->eventPhase != DOM_EVENT_CAPTURING_PHASE ||
			evt->target->parentNode != evt->relatedNode) {
		return;
	}

	DOM_EventTarget_removeEventListener(evt->target, "DOMNodeInserted", copy->?, nodeInserted, 1);
	DOM_EventTarget_removeEventListener(evt->target, "DOMNodeRemoved", evt->target, nodeRemoved, 1);

//MSG("%s %p %p", d->copy->nodeName, d->copy->parentNode, d->copy);
	if (DOM_Node_removeChild(d->copy->parentNode, d->copy) == NULL) {
		MSG("Failed to remove child");
	}
}

int
main(int argc, char *argv[])
{
	DOM_Document *d0, *d1;

	if (argc < 2) {
		MSG("Must provide XML filename");
		return EXIT_FAILURE;
	}
	if (!setlocale(LC_CTYPE, "")) {
			MSG("Can't set locale");
		return EXIT_FAILURE;
	}

	d0 = DOM_Implementation_createDocument(NULL, NULL, NULL);
	d1 = DOM_Implementation_createDocument(NULL, NULL, NULL);

	DOM_EventTarget_addEventListener(d0, "DOMNodeInserted", d1, nodeInserted, 1);
	DOM_EventTarget_addEventListener(d0, "DOMNodeRemoved", d1, nodeRemoved, 1);

	if (DOM_DocumentLS_load(d0, argv[1]) == 0) {
		if (DOM_Exception) {
			MSG("Failed to load: %s", argv[1]);
		}
		return EXIT_FAILURE;
	}

	DOM_DocumentLS_fwrite(d1, stdout);

	DOM_Document_destroyNode(d0, d0);
	DOM_Document_destroyNode(d1, d1);

	return EXIT_SUCCESS;
}

