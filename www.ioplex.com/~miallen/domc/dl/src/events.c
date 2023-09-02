/* DOMC Document Object Model library in C
 * Copyright (c) 2001 Michael B. Allen <mba2000 ioplex.com>
 *
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* events.c
 */

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "defines.h"
#include <mba/msgno.h>
#include "mbs.h"
#include "domc.h"
#include "dom.h"

extern uint64_t timestamp(void);

/* DOM_DocumentEvent - Introduced in DOM Level 2
 */

void
DOM_DocumentEvent_destroyEvent(DOM_DocumentEvent *doc, DOM_Event *evt)
{
	if (doc && evt) {
		if (evt->type) {
			/*free(evt->type); */
		}
		free(evt);
	}
}

DOM_Event *
DOM_DocumentEvent_createEvent(DOM_DocumentEvent *doc, const DOM_String *eventType)
{
	DOM_Event *evt;

	if (doc == NULL || eventType == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (strcmp(eventType, "Events") == 0 ||
					strcmp(eventType, "UIEvents") == 0 ||
					strcmp(eventType, "TextEvents") == 0) {
		evt = calloc(sizeof *evt, 1);
		if (evt == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			return NULL;
		}
	} else {
		DOM_Exception = DOM_NOT_SUPPORTED_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	return evt;
}

/* DOM_Event - Introduced in DOM Level 2
 */

void
DOM_Event_stopPropagation(DOM_Event *evt)
{
	if (evt) {
		evt->_sp = 1;
	}
}
void
DOM_Event_preventDefault(DOM_Event *evt)
{
	if (evt && evt->cancelable) {
		evt->_pd = 1;
	}
}
void
DOM_Event_initEvent(DOM_Event *evt, const DOM_String *eventTypeArg,
										int canBubbleArg, int cancelableArg)
{
	if (evt == NULL || eventTypeArg == NULL || *eventTypeArg == '\0') {
		return;
	}
	evt->type = eventTypeArg; /* no dup? */
	evt->bubbles = canBubbleArg;
	evt->cancelable = cancelableArg;
}

/* DOM_UIEvent
 */

void
DOM_UIEvent_initUIEvent(DOM_UIEvent *evt, const DOM_String *typeArg,
										int canBubbleArg, int cancelableArg,
										DOM_AbstractView *viewArg, long detailArg)
{
	if (evt == NULL || typeArg == NULL || *typeArg == '\0') {
		return;
	}
	DOM_Event_initEvent(evt, typeArg, canBubbleArg, cancelableArg);
	evt->view = viewArg;
	evt->detail = detailArg;
}

/* DOM_EventTarget - Introduced in DOM Level 2
 */

void
DOM_EventTarget_addEventListener(DOM_EventTarget *target, const DOM_String *type,
										DOM_EventListener *listener,
										DOM_EventListener_handleEvent listener_fn,
										int useCapture)
{
	ListenerEntry *e;
	unsigned int i;
	int opos = -1;

	if (target == NULL || type == NULL || listener_fn == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}

	for (i = 0; i < target->listeners_len; i++) {
		e = target->listeners[i];                      /* skip duplicates */

		if (e == NULL) {
			if (opos == -1) {
				opos = i;             /* find open position for new entry */
			}                         /* really need a hash code for this */
		} else if (e->listener == listener && e->listener_fn == listener_fn &&
					e->useCapture == useCapture && strcmp(e->type, type) == 0) {
			return;
		}
	}

	if ((e = malloc(sizeof *e)) == NULL ||
							(e->type = mbsdup(type)) == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		free(e);
		return;
	}
	e->listener = listener;
	e->listener_fn = listener_fn;
	e->useCapture = useCapture;

	if (opos == -1) {
		target->listeners = realloc(target->listeners,
										sizeof *e * (target->listeners_len + 1));
		if (target->listeners == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			free(e);
			return;
		}
		target->listeners[target->listeners_len++] = e;
	} else {
		target->listeners[opos] = e;
	}
/*MSG("added listener: type=%s,target=%s,useCapture=%d\n", type, target->nodeName, useCapture);
 */
}
void
DOM_EventTarget_removeEventListener(DOM_EventTarget *target, const DOM_String *type,
										DOM_EventListener *listener,
										DOM_EventListener_handleEvent listener_fn,
										int useCapture)
{
	ListenerEntry *e;
	unsigned int i;

	if (target == NULL || type == NULL || listener_fn == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}

	for (i = 0; i < target->listeners_len; i++) {
		e = target->listeners[i];

		if (e && e->listener == listener && e->listener_fn == listener_fn &&
					e->useCapture == useCapture && strcmp(e->type, type) == 0) {
			target->listeners[i] = NULL;
			free(e->type);
			free(e);
			return;
		}
	}
}
static void
trigger(DOM_EventTarget *target, DOM_Event *evt, int useCapture)
{
	ListenerEntry *e;
	unsigned int j, lcount;

	if (target && target->listeners_len && evt->_sp == 0) {
		DOM_EventListener_handleEvent *cpy_of_listener_fns;

		if ((cpy_of_listener_fns = malloc(target->listeners_len *
						sizeof(*cpy_of_listener_fns))) == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			return;
		}

		lcount = target->listeners_len;        /* copy listeners */
		for (j = 0; j < lcount; j++) {
			e = target->listeners[j];
			cpy_of_listener_fns[j] = e ? e->listener_fn : NULL;
		}

		evt->currentTarget = target;
		for (j = 0; j < lcount; j++) {
			e = target->listeners[j]; /* If the entry is NULL, the listener has
                          * since been removed and is therefore skipped. If it
                          * is not NULL but the listeners do not match, then
                          * it was removed but another listener was added in
                          * its place and therefore should be skipped. However,
                          * if a listener is removed but then added again
                          * while the current set is still being processed
                          * there is a chance that it will be tiggered if it
                          * meets the criteria tested below. Regardless of
                          * the slim chances of this occuring (have to be the
                          * same listener added to the same array) it may
						  * need to be addressed.
                          */
/*MSG("e=%p,e->listener_fn=%p,cpy_of_listener_fns[%d]=%p,e->useCapture=%d,e->type=%s,evt->type=%s", e, e->listener_fn, j, cpy_of_listener_fns[j], e->useCapture, e->type, evt->type);
 */

			if (e && e->listener_fn == cpy_of_listener_fns[j] &&
					e->useCapture == useCapture && strcmp(e->type, evt->type) == 0) {
				e->listener_fn(e->listener, evt); /* invoke the listener function */
			}
		}
		free(cpy_of_listener_fns);
	}
}
int
DOM_EventTarget_dispatchEvent(DOM_EventTarget *target, DOM_Event *evt)
{
	DOM_EventTarget **targets, *t;
	unsigned int tcount, i;

	if (target == NULL || evt == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return 1;
	}
/*MSG("type=%s,target=%s,listeners_len=%d", evt->type, target->nodeName, target->listeners_len);
 */

	if (evt->type == NULL || *evt->type == '\0') {
		DOM_Exception = DOM_UNSPECIFIED_EVENT_TYPE_ERR;
		PMNO(DOM_Exception);
		return 1;
	}

	targets = NULL;
	evt->target = target;							/* post-initialization */
	evt->timeStamp = timestamp();
	evt->_sp = 0;
	evt->_pd = 0;

	tcount = 0;									/* count targets/ancestors */
	for (t = target->parentNode; t; t = t->parentNode) {
		tcount++;
	}
	if (tcount) {
		targets = malloc(sizeof *targets * tcount);
		if (targets == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			return 1;
		}
	}
	i = tcount;						/* save state of tree in targets array */
	for (t = target->parentNode; t; t = t->parentNode) {
		targets[--i] = t;
	}
												/* Trigger capturers
												 */
	evt->eventPhase = DOM_EVENT_CAPTURING_PHASE;
	for (i = 0; i < tcount && evt->_sp == 0; i++) {
		trigger(targets[i], evt, 1);
	}
												/* Trigger regular listeners
											 	 */
	evt->eventPhase = DOM_EVENT_AT_TARGET;
	trigger(target, evt, 0);

												/* Trigger bubblers
											 	 */
	evt->eventPhase = DOM_EVENT_BUBBLING_PHASE;
	i = tcount;
	while (i-- && evt->bubbles && evt->_sp == 0) {
		trigger(targets[i], evt, 0);
	}

	if (targets) {
		free(targets);
	}
	return !evt->_pd;
}

/* DOM_TextEvent - Introduced in DOM Level 3
 * http://www.w3.org/TR/2002/WD-DOM-Level-3-Events-20020208/events.html
 */


int
DOM_TextEvent_checkModifier(DOM_TextEvent *evt, unsigned int modifier)
{
	return evt->_modifiers & (1 << (modifier - 1));
}
void
DOM_TextEvent_initTextEvent(DOM_TextEvent *evt, const DOM_String *typeArg,
										int canBubbleArg, int cancelableArg,
										DOM_AbstractView *viewArg, long detailArg,
										DOM_String *outputStringArg,
										unsigned int keyValArg,
										unsigned int virtKeyValArg,
										int visibleOutputGeneratedArg,
										int numPadArg)
{
	if (evt == NULL || typeArg == NULL || *typeArg == '\0') {
		return;
	}
	DOM_UIEvent_initUIEvent(evt, typeArg, canBubbleArg, cancelableArg, viewArg, detailArg);
	evt->outputString = outputStringArg;
	evt->keyVal = keyValArg;
	evt->virtKeyVal = virtKeyValArg;
	evt->visibleOutputGenerated = visibleOutputGeneratedArg;
	evt->numPad = numPadArg;
}
void
DOM_TextEvent_initModifier(DOM_TextEvent *evt, unsigned int modifier, int value)
{
	if (evt && modifier > 0 && modifier <= DOM_VK_RIGHT_META) {
		if (value) {
			evt->_modifiers |= 1 << (modifier - 1);
		} else {
			evt->_modifiers &= ~(1 << (modifier - 1));
		}
	}
}

/* MutationEvent
 */

void
DOM_MutationEvent_initMutationEvent(DOM_MutationEvent *evt, DOM_String *typeArg,
								int canBubbleArg, int cancelableArg,
								DOM_Node *relatedNodeArg,
								DOM_String *prevValueArg,
								DOM_String *newValueArg,
								DOM_String *attrNameArg,
								unsigned short attrChangeArg)
{
	if (evt == NULL || typeArg == NULL || *typeArg == '\0') {
		return;
	}
	DOM_Event_initEvent(evt, typeArg, canBubbleArg, cancelableArg);
	evt->relatedNode = relatedNodeArg;
	evt->prevValue = prevValueArg;
	evt->newValue = newValueArg;
	evt->attrName = attrNameArg;
	evt->attrChange = attrChangeArg;
}
void
updateCommonParent(DOM_Node *node)
{
	DOM_Node *n, *cp;

	if (node == NULL || node->ownerDocument == NULL) {
		return;
	}
	if (node->ownerDocument->u.Document.commonParent == NULL) {
		node->ownerDocument->u.Document.commonParent = node;
		return;
	}

	cp = NULL;
	for (n = node; n; n = n->parentNode) {
		if (n == node->ownerDocument->u.Document.commonParent) {
			return;
		} else if (cp == NULL && n->subtreeModified == 1) {
			cp = n;
		} else {
			n->subtreeModified = 1;
		}
	}

	node->ownerDocument->u.Document.commonParent = cp;
}
static void
_clearSubtreeModified(DOM_Document *doc)
{
	DOM_Node *n;

	for (n = doc->firstChild; n != NULL; n = n->nextSibling) {
		if (n->subtreeModified) {
			n->subtreeModified = 0;
			_clearSubtreeModified(n);
		}
	}
	doc->u.Document.commonParent = NULL;
}
void
DOM_MutationEvent_processSubtreeModified(DOM_Document *doc)
{
	DOM_Node *target;
	DOM_MutationEvent evt;

	if (doc->u.Document.commonParent == NULL) {
		return;
	}

	target = doc->u.Document.commonParent;
	_clearSubtreeModified(doc);

	DOM_MutationEvent_initMutationEvent(&evt, "DOMSubtreeModified",
				1, 0, NULL, NULL, NULL, NULL, 0);
	DOM_EventTarget_dispatchEvent(target, &evt);
}

