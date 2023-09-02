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

/* node.c - DOM_NodeList, DOM_NamedNodeMap, and DOM_Node
 */

#include <string.h>
#include <wchar.h>
#include "defines.h"
#include <mba/msgno.h>
#include "mbs.h"
#include "domc.h"
#include "dom.h"

/* Forward references
 */

void DOM_Document_destroyNamedNodeMap(DOM_Document *doc, DOM_NamedNodeMap *nnm, int free_nodes);
void updateCommonParent(DOM_Node *node);

/* Node
 */

void
DOM_Document_destroyNode(DOM_Document *doc, DOM_Node *node)
{
	if (node == NULL) {
        return;
    }

    if (node->childNodes) {
        DOM_Document_destroyNodeList(doc, node->childNodes, 1);
    }
	if (node->listeners) {
		unsigned int i;

		for (i = 0; i < node->listeners_len; i++) {
			if (node->listeners[i]) {
				free(node->listeners[i]->type);
				free(node->listeners[i]);
			}
		}
		free(node->listeners);
	}

    switch(node->nodeType) {
        case DOM_ELEMENT_NODE:
			DOM_Document_destroyNamedNodeMap(doc, node->attributes, 1);
            free(node->nodeName);
            break;
        case DOM_TEXT_NODE:
        case DOM_COMMENT_NODE:
        case DOM_CDATA_SECTION_NODE:
            free(node->nodeValue);
            break;
        case DOM_ATTRIBUTE_NODE:
            free(node->nodeName);
            free(node->nodeValue);
            break;
        case DOM_ENTITY_REFERENCE_NODE:
        case DOM_ENTITY_NODE:
			free(node->nodeName);
			free(node->nodeValue);
			free(node->u.Entity.publicId);
			free(node->u.Entity.systemId);
			free(node->u.Entity.notationName);
            break;
        case DOM_PROCESSING_INSTRUCTION_NODE:
            free(node->nodeName);
            free(node->nodeValue);
            break;
        case DOM_DOCUMENT_NODE:
			free(node->u.Document.version);
			free(node->u.Document.encoding);
			break;
        case DOM_DOCUMENT_TYPE_NODE:
			DOM_Document_destroyNamedNodeMap(doc, node->u.DocumentType.entities, 0);
			DOM_Document_destroyNamedNodeMap(doc, node->u.DocumentType.notations, 0);
			free(node->u.DocumentType.publicId);
			free(node->u.DocumentType.systemId);
			free(node->nodeName);
			break;
        case DOM_NOTATION_NODE:
			free(node->nodeName);
			free(node->u.Notation.publicId);
			free(node->u.Notation.systemId);
            break;
    }
    free(node);
}
DOM_Node *
Document_createNode(DOM_Document *doc, unsigned short nodeType)
{
    DOM_Node *node;

	msgno_add_codes(dom_codes);

	if (doc == NULL && nodeType != DOM_DOCUMENT_NODE && nodeType != DOM_DOCUMENT_TYPE_NODE) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNF(DOM_Exception, ": doc=NULL,nodeType=%u", nodeType);
		return NULL;
	}

    node = calloc(sizeof *node, 1);
    if (node == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		return NULL;
	}

    node->nodeType = nodeType;
    node->ownerDocument = doc;
    switch (nodeType) {
        case DOM_DOCUMENT_NODE:
        case DOM_DOCUMENT_TYPE_NODE:
			/* DocumentType doesn't really have children but we need to store DTD
			 * entries other than entities and notations */
        case DOM_ELEMENT_NODE:
        case DOM_ATTRIBUTE_NODE:
        case DOM_ENTITY_REFERENCE_NODE:
        case DOM_ENTITY_NODE:
        case DOM_DOCUMENT_FRAGMENT_NODE:
            node->childNodes = Document_createNodeList(doc);
            if (node->childNodes == NULL) {
				AMNO(DOM_CREATE_FAILED);
                DOM_Document_destroyNode(doc, node);
                return NULL;
            }
    }

    return node;
}

static int
_isAncestor(DOM_Node *node, DOM_Node *parent)
{
	DOM_Node *p;

	for (p = parent; p; p = p->parentNode) {
		if (p == node) {
			return 1;
		}
	}
	return 0;
}

static void
dispatchEventPreorder(DOM_Node *node, DOM_MutationEvent *evt)
{
	DOM_Node *n;

	DOM_EventTarget_dispatchEvent(node, evt);

	for (n = node->firstChild; n != NULL; n = n->nextSibling) {
		dispatchEventPreorder(n, evt);
	}
}
static void
dispatchEventPostorder(DOM_Node *node, DOM_MutationEvent *evt)
{
	DOM_Node *n;

	for (n = node->firstChild; n != NULL; n = n->nextSibling) {
		dispatchEventPostorder(n, evt);
	}

	DOM_EventTarget_dispatchEvent(node, evt);
}
static DOM_Node *
_removeChild(DOM_Node *node, DOM_Node *oldChild)
{
	NodeEntry *e;
	DOM_MutationEvent evt;

	if (NodeList_exists(node->childNodes, oldChild) == 0) {
		return NULL;
	}

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeRemoved", 1, 0, node, NULL, NULL, NULL, 0);
	DOM_EventTarget_dispatchEvent(oldChild, &evt);

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeRemovedFromDocument", 0, 0, NULL, NULL, NULL, NULL, 0);
	dispatchEventPostorder(oldChild, &evt);

	e = NodeList_remove(node->childNodes, oldChild);
	free(e);

	if (node->firstChild == node->lastChild) {
		node->firstChild = node->lastChild = NULL;
	} else if (oldChild == node->firstChild) {
		node->firstChild = oldChild->nextSibling;
		node->firstChild->previousSibling = NULL;
	} else if (oldChild == node->lastChild) {
		node->lastChild = oldChild->previousSibling;
		node->lastChild->nextSibling = NULL;
	} else {
        oldChild->previousSibling->nextSibling = oldChild->nextSibling;
        oldChild->nextSibling->previousSibling = oldChild->previousSibling;
	}

    oldChild->previousSibling = NULL;
    oldChild->nextSibling = NULL;
    oldChild->parentNode = NULL;

	if (MODIFYING_DOC_ELEM(node, oldChild)) {
		node->u.Document.documentElement = NULL;
	} else if (MODIFYING_DOCTYPE_ELEM(node, oldChild)) {
		node->u.Document.doctype = NULL;
		oldChild->ownerDocument = NULL;
	} else {
		updateCommonParent(node);
	}

    return oldChild;
}
DOM_Node *
DOM_Node_insertBefore(DOM_Node *node, DOM_Node *newChild, DOM_Node *refChild)
{
	NodeEntry *e;
	DOM_MutationEvent evt;

    if (node == NULL || newChild == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (newChild->ownerDocument != node->ownerDocument &&
				newChild->ownerDocument != node) {
		DOM_Exception = DOM_WRONG_DOCUMENT_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (refChild != NULL && refChild->parentNode != node) {
		DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if (newChild->nodeType == DOM_DOCUMENT_FRAGMENT_NODE) {
		DOM_Node *n, *nxt;

		for (n = newChild->firstChild; n != NULL; n = n->nextSibling) {
			if (INVALID_HIER_REQ(node, n) || _isAncestor(n, node)) {
				DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
				PMNO(DOM_Exception);
				return NULL;
			}
		}
		for (n = newChild->firstChild; n != NULL; n = nxt) {
			nxt = n->nextSibling;
			if (_removeChild(newChild, n) == NULL) {
				return NULL;
			}
			if (DOM_Node_insertBefore(node, n, refChild) == NULL) {
				DOM_Document_destroyNode(n->ownerDocument, n);
				return NULL;
			}
		}
		return newChild;
	}
	if (INVALID_HIER_REQ(node, newChild) || _isAncestor(newChild, node)) {
		DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	_removeChild(node, newChild);

	if ((e = NodeList_insert(node->childNodes, newChild, refChild)) == NULL) {
        return NULL;
    }

	if (node->firstChild == NULL) {
		node->firstChild = node->lastChild = newChild;
		newChild->previousSibling = NULL;
    	newChild->nextSibling = NULL;
	} else if (refChild == NULL) {
		newChild->previousSibling = node->lastChild;
		node->lastChild->nextSibling = newChild;
		node->lastChild = newChild;
    	newChild->nextSibling = NULL;
	} else {
		newChild->previousSibling = refChild->previousSibling;
		newChild->nextSibling = refChild;
		if (refChild == node->firstChild) {
			node->firstChild = newChild;
			newChild->previousSibling = NULL;
		} else {
			refChild->previousSibling->nextSibling = newChild;
		}
		refChild->previousSibling = newChild;
	}
    newChild->parentNode = node;

	if (MODIFYING_DOC_ELEM(node, newChild)) {
		node->u.Document.documentElement = newChild;
	} else if (MODIFYING_DOCTYPE_ELEM(node, newChild)) {
		node->u.Document.doctype = newChild;
		newChild->ownerDocument = node;
	}

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeInserted", 1, 0, node, NULL, NULL, NULL, 0);
	DOM_EventTarget_dispatchEvent(newChild, &evt);

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeInsertedIntoDocument", 0, 0, NULL, NULL, NULL, NULL, 0);
	dispatchEventPreorder(newChild, &evt);

	updateCommonParent(node);

    return newChild;
}
DOM_Node *
DOM_Node_replaceChild(DOM_Node *node, DOM_Node *newChild, DOM_Node *oldChild)
{
	DOM_MutationEvent evt;

    if (node == NULL || newChild == NULL || oldChild == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (newChild->ownerDocument != node->ownerDocument &&
				newChild->ownerDocument != node) {
		DOM_Exception = DOM_WRONG_DOCUMENT_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if (!NodeList_exists(node->childNodes, oldChild)) {
		DOM_Exception = DOM_NOT_FOUND_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if (newChild->nodeType == DOM_DOCUMENT_FRAGMENT_NODE) {
		DOM_Node *n, *nxt;

		for (n = newChild->firstChild; n != NULL; n = n->nextSibling) {
			if (INVALID_HIER_REQ(node, n) || _isAncestor(n, node)) {
				DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
				PMNO(DOM_Exception);
				return NULL;
			}
		}
		for (n = newChild->firstChild; n != NULL; n = nxt) {
			nxt = n->nextSibling;
			if (_removeChild(newChild, n) == NULL) {
				return NULL;
			}
			if (DOM_Node_insertBefore(node, n, oldChild) == NULL) {
				DOM_Document_destroyNode(n->ownerDocument, n);
				return NULL;
			}
		}

		if (_removeChild(node, oldChild) == NULL) {
			return NULL;
		}

		return oldChild;
	}
	if (INVALID_HIER_REQ(node, newChild) || _isAncestor(newChild, node)) {
		DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	_removeChild(node, newChild);

	if (NodeList_exists(node->childNodes, oldChild) == 0) {
		return NULL;
	}

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeRemoved", 1, 0, node, NULL, NULL, NULL, 0);
	DOM_EventTarget_dispatchEvent(oldChild, &evt);

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeRemovedFromDocument", 0, 0, NULL, NULL, NULL, NULL, 0);
	dispatchEventPostorder(oldChild, &evt);

	NodeList_replace(node->childNodes, newChild, oldChild);

	node->firstChild = node->childNodes->first->node;
	node->lastChild = node->childNodes->last->node;

    if ((newChild->previousSibling = oldChild->previousSibling)) {
        newChild->previousSibling->nextSibling = newChild;
    }
    if ((newChild->nextSibling = oldChild->nextSibling)) {
        newChild->nextSibling->previousSibling = newChild;
    }

    newChild->parentNode = node;
    oldChild->parentNode = NULL;
    oldChild->previousSibling = NULL;
    oldChild->nextSibling = NULL;

	if (MODIFYING_DOC_ELEM(node, newChild)) {
		node->u.Document.documentElement = newChild;
	} else if (MODIFYING_DOCTYPE_ELEM(node, newChild)) {
		node->u.Document.doctype = newChild;
		newChild->ownerDocument = node;
	}

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeInserted", 1, 0, node, NULL, NULL, NULL, 0);
	DOM_EventTarget_dispatchEvent(newChild, &evt);

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeInsertedIntoDocument", 0, 0, NULL, NULL, NULL, NULL, 0);
	dispatchEventPreorder(newChild, &evt);

	updateCommonParent(node);

    return oldChild;
}
DOM_Node *
DOM_Node_removeChild(DOM_Node *node, DOM_Node *oldChild)
{
    if (node == NULL || oldChild == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (oldChild->ownerDocument != node->ownerDocument &&
				oldChild->ownerDocument != node) {
		DOM_Exception = DOM_WRONG_DOCUMENT_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if ((oldChild = _removeChild(node, oldChild)) == NULL) {
		DOM_Exception = DOM_NOT_FOUND_ERR;
		PMNO(DOM_Exception);
	}

    return oldChild;
}
DOM_Node *
DOM_Node_appendChild(DOM_Node *node, DOM_Node *newChild)
{
	DOM_MutationEvent evt;

    if (node == NULL || newChild == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
        return NULL;
    }
	if (newChild->ownerDocument != node->ownerDocument &&
				node->nodeType != DOM_DOCUMENT_NODE &&
				newChild->nodeType != DOM_DOCUMENT_TYPE_NODE) {
		DOM_Exception = DOM_WRONG_DOCUMENT_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if (newChild->nodeType == DOM_DOCUMENT_FRAGMENT_NODE) {
		DOM_Node *n, *nxt;

		for (n = newChild->firstChild; n != NULL; n = n->nextSibling) {
			if (INVALID_HIER_REQ(node, n) || _isAncestor(n, node)) {
				DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
				PMNO(DOM_Exception);
				return NULL;
			}
		}
		for (n = newChild->firstChild; n != NULL; n = nxt) {
			nxt = n->nextSibling;
			if (_removeChild(newChild, n) == NULL) {
				return NULL;
			}
			if (DOM_Node_appendChild(node, n) == NULL) {
				DOM_Document_destroyNode(n->ownerDocument, n);
				return NULL;
			}
		}
		return newChild;
	}
	if (INVALID_HIER_REQ(node, newChild) || _isAncestor(newChild, node)) {
		DOM_Exception = DOM_HIERARCHY_REQUEST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	_removeChild(node, newChild);

	if (NodeList_append(node->childNodes, newChild) == NULL) {
		return NULL;
	}

	if (node->firstChild == NULL) {
		node->firstChild = node->lastChild = newChild;
		newChild->previousSibling = NULL;
		newChild->nextSibling = NULL;
	} else {
		node->lastChild->nextSibling = newChild;
		newChild->previousSibling = node->lastChild;
		node->lastChild = newChild;
	}
    newChild->nextSibling = NULL;
    newChild->parentNode = node;

	if (MODIFYING_DOC_ELEM(node, newChild)) {
		node->u.Document.documentElement = newChild;
	} else if (MODIFYING_DOCTYPE_ELEM(node, newChild)) {
		node->u.Document.doctype = newChild;
		newChild->ownerDocument = node;
	}

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeInserted", 1, 0, node, NULL, NULL, NULL, 0);
	DOM_EventTarget_dispatchEvent(newChild, &evt);

	DOM_MutationEvent_initMutationEvent(&evt,
				"DOMNodeInsertedIntoDocument", 0, 0, NULL, NULL, NULL, NULL, 0);
	dispatchEventPreorder(newChild, &evt);

	updateCommonParent(node);

    return newChild;
}
int
DOM_Node_hasChildNodes(const DOM_Node *node)
{
    return node != NULL && node->firstChild;
}
extern const char *node_names[];
static DOM_Node *
Node_cloneNode(DOM_Document *ownerDocument, DOM_Node *node, int deep)
{
    DOM_Node *clone = NULL;
	DOM_Node *ntmp, *ctmp;
	NodeEntry *e;
	DOM_String *tmp;

    switch(node->nodeType) {
        case DOM_ELEMENT_NODE:
			clone = DOM_Document_createElement(ownerDocument, node->nodeName);
			if (clone) {
				for (e = node->attributes->first; e != NULL; e = e->next) {
					if ((ctmp = Node_cloneNode(ownerDocument, e->node, deep)) == NULL ||
									NodeList_append(clone->attributes, ctmp) == NULL) {
						DOM_Document_destroyNode(clone->ownerDocument, ctmp);
						DOM_Document_destroyNode(clone->ownerDocument, clone);
						return NULL;
					}
				}
			}
            break;
        case DOM_ATTRIBUTE_NODE:
			if ((clone = DOM_Document_createAttribute(ownerDocument, node->nodeName))) {
           		clone->u.Attr.specified = node->u.Attr.specified;
				free(clone->nodeValue);
           		clone->u.Attr.value = clone->nodeValue = mbsdup(node->nodeValue);
				if (clone->u.Attr.value == NULL) {
					DOM_Exception = errno;
					PMNO(DOM_Exception);
					DOM_Document_destroyNode(clone->ownerDocument, clone);
					return NULL;
				}
			}
            break;
		case DOM_COMMENT_NODE:
			clone = DOM_Document_createComment(ownerDocument, node->nodeValue);
			break;
		case DOM_TEXT_NODE:
			clone = DOM_Document_createTextNode(ownerDocument, node->nodeValue);
			break;
		case DOM_CDATA_SECTION_NODE:
			clone = DOM_Document_createCDATASection(ownerDocument, node->nodeValue);
			break;
		case DOM_DOCUMENT_FRAGMENT_NODE:
			clone = DOM_Document_createDocumentFragment(ownerDocument);
			break;
		case DOM_DOCUMENT_NODE:
			clone = ownerDocument;
			break;
        case DOM_PROCESSING_INSTRUCTION_NODE:
			clone = DOM_Document_createProcessingInstruction(ownerDocument,
						node->u.ProcessingInstruction.target,
						node->u.ProcessingInstruction.data);
			break;
        case DOM_ENTITY_NODE:
			if ((clone = Document_createNode(ownerDocument, DOM_ENTITY_NODE))) {
				tmp = node->nodeValue;
				if ((clone->nodeName = mbsdup(node->nodeName)) == NULL ||
                       /* This will eventually go away as Entities should not have nodeValues
                        */
						(clone->nodeValue = mbsdup(node->nodeValue)) == NULL ||
						(node->u.Entity.publicId &&
						(clone->u.Entity.publicId = mbsdup(node->u.Entity.publicId)) == NULL) ||
						(node->u.Entity.systemId &&
						(clone->u.Entity.systemId = mbsdup(node->u.Entity.systemId)) == NULL) ||
						(node->u.Entity.notationName &&
						(clone->u.Entity.notationName = mbsdup(node->u.Entity.notationName)) == NULL)) {
					DOM_Exception = errno;
					PMNO(DOM_Exception);
					DOM_Document_destroyNode(clone->ownerDocument, clone);
					return NULL;
				}
				free(tmp);
			}
			break;
        case DOM_NOTATION_NODE:
			if ((clone = Document_createNode(ownerDocument, DOM_NOTATION_NODE))) {
				if ((clone->nodeName = mbsdup(node->nodeName)) == NULL ||
						(node->u.Notation.publicId &&
						(clone->u.Notation.publicId = mbsdup(node->u.Notation.publicId)) == NULL) ||
						(node->u.Notation.systemId &&
						(clone->u.Notation.systemId = mbsdup(node->u.Notation.systemId)) == NULL)) {
					DOM_Exception = errno;
					PMNO(DOM_Exception);
					DOM_Document_destroyNode(clone->ownerDocument, clone);
					return NULL;
				}
			}
			break;
        case DOM_DOCUMENT_TYPE_NODE:
			if ((clone = DOM_Implementation_createDocumentType(node->nodeName, NULL, NULL))) {
				if ((node->u.DocumentType.publicId &&
						(clone->u.DocumentType.publicId = mbsdup(node->u.DocumentType.publicId)) == NULL) ||
						(node->u.DocumentType.systemId &&
						(clone->u.DocumentType.systemId = mbsdup(node->u.DocumentType.systemId)) == NULL)) {
					DOM_Exception = errno;
					PMNO(DOM_Exception);
					DOM_Document_destroyNode(clone->ownerDocument, clone);
					return NULL;
				}
			}
			ownerDocument->u.Document.doctype = clone;
			clone->ownerDocument = ownerDocument;
			break;
        case DOM_ENTITY_REFERENCE_NODE:
			DOM_Exception = DOM_NOT_SUPPORTED_ERR;
			PMNO(DOM_Exception);
			return NULL;
    }

	if (deep && clone && node->childNodes) {
		for (ntmp = node->firstChild; ntmp != NULL; ntmp = ntmp->nextSibling) {
			ctmp = Node_cloneNode(ownerDocument, ntmp, 1);
			if (ctmp == NULL || DOM_Node_appendChild(clone, ctmp) == NULL) {
				DOM_Document_destroyNode(clone->ownerDocument, ctmp);
				DOM_Document_destroyNode(clone->ownerDocument, clone);
				return NULL;
			}
		}
	}

    return clone;
}
DOM_Node *
DOM_Node_cloneNode(DOM_Node *node, int deep)
{
	if (node == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (node->nodeType == DOM_DOCUMENT_NODE) {
		DOM_Document *doc;
		if ((doc = DOM_Implementation_createDocument(NULL, NULL, NULL)) == NULL) {
			AMSG("");
			return NULL;
		}
		return Node_cloneNode(doc, node, deep);
	}
	return Node_cloneNode(node->ownerDocument, node, deep);
}
DOM_String *
DOM_Node_getNodeValue(DOM_Node *node)
{
	if (node == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	return node->nodeValue;
}
void
DOM_Node_setNodeValue(DOM_Node *node, DOM_String *value)
{
	DOM_String *str = NULL;

	if (node == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}
    switch(node->nodeType) {
        case DOM_ATTRIBUTE_NODE:
		case DOM_COMMENT_NODE:
		case DOM_TEXT_NODE:
		case DOM_CDATA_SECTION_NODE:
        case DOM_PROCESSING_INSTRUCTION_NODE:
			if ((str = mbsdup(value)) == NULL) {
				DOM_Exception = errno;
				AMSG("");
				return;
			}
			break;
	}
    switch(node->nodeType) {
        case DOM_ATTRIBUTE_NODE:
			free(node->nodeValue);
			node->nodeValue = node->u.Attr.value = str;
			break;
		case DOM_COMMENT_NODE:
		case DOM_TEXT_NODE:
		case DOM_CDATA_SECTION_NODE:
			free(node->nodeValue);
			node->nodeValue = node->u.CharacterData.data = str;
			break;
        case DOM_PROCESSING_INSTRUCTION_NODE:
			free(node->nodeValue);
			node->nodeValue = node->u.ProcessingInstruction.data = str;
			break;
		default:
			return; /* No effect */
	}
}

