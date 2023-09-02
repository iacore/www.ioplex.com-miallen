#include <stdlib.h>
#include <mba/msgno.h>
#include <domc.h>

static const char *cnames[] = {
	"",
	"MODIFICATION",
	"ADDITION",
	"REMOVAL"
};

void
handleEvent(void *this, DOM_Event *evt)
{
	MSG("Event fired: type=%s,target=%s,currentTarget=%s,eventPhase=%d,\n  bubbles=%d,relatedNode=%s,prevValue=%s,newValue=%s,attrName=%s,attrChange=%s", evt->type, evt->target->nodeName, evt->currentTarget->nodeName, evt->eventPhase, evt->bubbles, evt->relatedNode->nodeName, evt->prevValue ? evt->prevValue : "NULL", evt->newValue ? evt->newValue : "NULL", evt->attrName ? evt->attrName : "NULL", cnames[evt->attrChange]);
}
void
Document_handleEvent(void *this, DOM_Event *evt)
{
	MSG("Document event: %s: %s", evt->type, evt->target->nodeName);
}

int
main(int argc, char *argv[])
{
	DOM_Document *doc;
	DOM_Element *p, *three, *foo, *bar;

	if (argc < 2) {
		MSG("Must provide XML filename");
		return EXIT_FAILURE;
	}

	doc = DOM_Implementation_createDocument(NULL, NULL, NULL);
	if (DOM_DocumentLS_load(doc, argv[1]) == 0) {
		if (DOM_Exception) {
			MNO(DOM_Exception);
		}
		return EXIT_FAILURE;
	}

	p = DOM_NodeList_item(DOM_Document_getElementsByTagName(doc, "two"), 0);
	three = DOM_NodeList_item(p->childNodes, 1);
	foo = DOM_Document_createElement(doc, "foo");
	bar = DOM_Document_createElement(doc, "bar");

	DOM_EventTarget_addEventListener(three, "DOMNodeRemoved", NULL, handleEvent, 0);
	DOM_EventTarget_addEventListener(three, "DOMNodeInserted", NULL, handleEvent, 0);
	DOM_EventTarget_addEventListener(foo, "DOMNodeRemoved", NULL, handleEvent, 0);
	DOM_EventTarget_addEventListener(foo, "DOMNodeInserted", NULL, handleEvent, 0);
	DOM_EventTarget_addEventListener(bar, "DOMNodeInserted", NULL, handleEvent, 0);
	DOM_EventTarget_addEventListener(bar, "DOMNodeRemovedFromDocument", NULL, Document_handleEvent, 0);

	DOM_Node_appendChild(p, foo);          /* three, foo */
	DOM_Node_removeChild(p, three);        /* foo        */
	DOM_Node_insertBefore(p, bar, foo);    /* bar, foo   */
	DOM_Node_replaceChild(p, three, foo);  /* bar, three */

	DOM_Node_removeChild(doc, doc->u.Document.documentElement);

	DOM_Document_destroyNode(doc, doc);

	return EXIT_SUCCESS;
}
