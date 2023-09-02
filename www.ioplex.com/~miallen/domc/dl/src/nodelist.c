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

/* nodelist.c - the DOM_NodeList interface
 */

#include "defines.h"
#include <mba/msgno.h>
#include "domc.h"
#include "dom.h"

#if FAST_NODELIST

/* The number of nodes required in a list before hashing starts */
#define FAST_FILLFACTOR 16

static void _removeFromMap(DOM_NodeList* nl, DOM_Node* key)
{
	if (nl->_map) {
		if (hashmap_get(nl->_map, key) != NULL) {
			void* d = NULL;
			void* k = key;
			hashmap_remove(nl->_map, &k, &d);
		}
	}
}

static int _addToMap(DOM_NodeList* nl, DOM_Node* key, NodeEntry* val)
{
	if (!nl->_map && nl->length > FAST_FILLFACTOR) {
		
		nl->_map = hashmap_new(0, NULL, NULL, NULL);
		
		/* Hash what we currently have */
		if (nl->_map) {
			NodeEntry *e = nl->first;
			while (e) {
				_addToMap(nl, e->node, e);
				e = e->next;
			}
		}
	}
	
	if (nl->_map) {
		_removeFromMap(nl, key);
			
		if (hashmap_put(nl->_map, key, val) == -1) {
			DOM_Exception = errno;
			return -1;
		}		
	}
	
	return 0;
}

#endif

static NodeEntry* _lookupNode(DOM_NodeList* nl, DOM_Node* node)
{
	NodeEntry* s;
	
#if FAST_NODELIST
	if (nl->_map)
		s = (NodeEntry*)hashmap_get(nl->_map, node);
	else 
#endif
		for (s = nl->first; s != NULL && s->node != node; s = s->next) {
			;
		}
		
	return s;
}

/* NodeList
 */

DOM_NodeList *
Document_createNodeList(DOM_Document *doc)
{
	DOM_NodeList *r;

	if ((r = calloc(sizeof *r, 1)) == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
	}
	r->_ownerDocument = doc;

	return r;
}
void
DOM_Document_destroyNodeList(DOM_Document *doc, DOM_NodeList *nl, int free_nodes)
{
	if (nl) {
		if (nl->filter == 0) {
			NodeEntry *e, *tmp;

			e = nl->first;
			while (e != NULL) {
				if (free_nodes) {
					DOM_Document_destroyNode(doc, e->node);
				}
				tmp = e;
				e = e->next;
				free(tmp); 
			}
		}

#if FAST_NODELIST
		if(nl->_map)		
			hashmap_del(nl->_map, NULL, NULL, NULL);
#endif
		
		free(nl);
	}
}

DOM_Node *
NodeList_itemFiltered(const DOM_NodeList *list, int index, unsigned short nodeType)
{
    if (list && index >= 0 && index < list->length) {
		NodeEntry *e;

        for (e = list->first; e != NULL; e = e->next) {
			if (e->node->nodeType == nodeType) {
				if (index == 0) {
					return e->node;
				}
				index--;
			}
        }
    }

    return NULL;
}
DOM_Node *
DOM_NodeList_item(const DOM_NodeList *list, int index)
{
    if (list) {
		if (list->filter) {
			return NodeList_itemFiltered(list->list, index, list->filter);
		}

		if (index >= 0 && index < list->length) {
			NodeEntry *e;

    	    for (e = list->first; e != NULL; e = e->next, index--) {
				if (index == 0) {
					return e->node;
				}
			}
		}
    }

    return NULL;
}
NodeEntry *
NodeList_insert(DOM_NodeList *nl, DOM_Node *newChild, DOM_Node *refChild)
{
	NodeEntry *e;
	NodeEntry *s = NULL;

	if (nl == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (nl->filter) {
		DOM_Exception = DOM_FILTERED_LIST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if(refChild != NULL)
	{
		s = _lookupNode(nl, refChild);
		if(s == NULL || s->node != refChild) {
			DOM_Exception = DOM_NOT_FOUND_ERR;
			PMNO(DOM_Exception);
			return NULL;
		}		
	}
	
	if ((e = calloc(sizeof *e, 1)) == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		return NULL;
	}

#if FAST_NODELIST
	if (_addToMap(nl, newChild, e) == -1) {
		PMNO(DOM_Exception);
		free(e);
		return NULL;
	}
#endif
	
	e->node = newChild;
	if (nl->length == 0) {
		nl->first = nl->last = e;
	} else if (refChild == NULL) {
		e->prev = nl->last;
		nl->last->next = e;
		nl->last = e;
	} else {
		e->prev = s->prev;
		e->next = s;
		if (s == nl->first) {
			nl->first = e;
		} else {
			s->prev->next = e;
		}
		s->prev = e;
	}
	nl->length++;

	/* If an attribute is being added this is probably a NamedNodeMap
	 * in which case we must set the ownerElement.
	 */
	if (newChild->nodeType == DOM_ATTRIBUTE_NODE) {
		newChild->u.Attr.ownerElement = nl->_ownerElement;
	}

	return e;
}
NodeEntry *
NodeList_replace(DOM_NodeList *nl, DOM_Node *newChild, DOM_Node *oldChild)
{
	NodeEntry *e;

	if (nl == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (nl->filter) {
		DOM_Exception = DOM_FILTERED_LIST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	e = _lookupNode(nl, oldChild);
	if (e == NULL) {
		DOM_Exception = DOM_NOT_FOUND_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

#if FAST_NODELIST
	_removeFromMap(nl, oldChild);
	if(_addToMap(nl, newChild, e) == -1) {
		PMNO(DOM_Exception);
		return NULL;
	}
#endif

	e->node = newChild;

	if (oldChild->nodeType == DOM_ATTRIBUTE_NODE) {
		oldChild->u.Attr.ownerElement = NULL;
	}

	return e;
}
NodeEntry *
NodeList_remove(DOM_NodeList *nl, DOM_Node *oldChild)
{
	NodeEntry *e;

	if (nl == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (nl->filter) {
		DOM_Exception = DOM_FILTERED_LIST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	e = _lookupNode(nl, oldChild);
	if (e == NULL) {
		return NULL;
	}
	
#if FAST_NODELIST
	_removeFromMap(nl, oldChild);	
#endif

	if (nl->first == nl->last) {
		nl->first = nl->last = NULL;
	} else if (e == nl->first) {
		nl->first = e->next;
		nl->first->prev = NULL;
	} else if (e == nl->last) {
		nl->last = e->prev;
		nl->last->next = NULL;
	} else {
		e->prev->next = e->next;
		e->next->prev = e->prev;
	}
	nl->length--;

	/* Decrement a filtered node list too? */

	if (oldChild->nodeType == DOM_ATTRIBUTE_NODE) {
		oldChild->u.Attr.ownerElement = NULL;
	}

	return e;
}
extern const char *node_names[];
NodeEntry *
NodeList_append(DOM_NodeList *nl, DOM_Node *newChild)
{
	NodeEntry *e;
	DOM_DocumentType *doctype;

	if (nl == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNF(DOM_Exception, ": %p", newChild);
		return NULL;
	}
	if (nl->filter) {
		DOM_Exception = DOM_FILTERED_LIST_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if ((e = calloc(sizeof *e, 1)) == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		return NULL;
	}

#if FAST_NODELIST
	if(_addToMap(nl, newChild, e) == -1) {
		PMNO(DOM_Exception);
		free(e);
		return NULL;
	}
#endif

	e->node = newChild;
	if (nl->first == NULL) {
		nl->first = nl->last = e;
	} else {
		nl->last->next = e;
		e->prev = nl->last;
		nl->last = e;
	}

	nl->length++;

	/* If the node list is the DocumentType children and a Notation
	 * or Entity is being added we must artificially update the length
	 * member of those filtered lists
	 */
	if (newChild->ownerDocument &&
					(doctype = newChild->ownerDocument->u.Document.doctype) &&
					nl == doctype->childNodes) {
		if (newChild->nodeType == DOM_NOTATION_NODE) {
			doctype->u.DocumentType.notations->length++;
		} else if (newChild->nodeType == DOM_ENTITY_NODE) {
			doctype->u.DocumentType.entities->length++;
		}
	}
	/* If an attribute is being added this is probably a NamedNodeMap
	 * in which case we must set the ownerElement.
	 */
	if (newChild->nodeType == DOM_ATTRIBUTE_NODE) {
		newChild->u.Attr.ownerElement = nl->_ownerElement;
	}

	return e;
}
int
NodeList_exists(DOM_NodeList *nl, DOM_Node *child)
{
	NodeEntry *e;

	if (nl == NULL || nl->filter) {
		return 0;
	}

	e = _lookupNode(nl, child);
	return e != NULL;
}

