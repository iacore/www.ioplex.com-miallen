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

/* namednodemap.c - the DOM_NamedNodeMap interface
 */

#include <string.h>
#include "defines.h"
#include <mba/msgno.h>
#include "domc.h"
#include "dom.h"

/* NamedNodeMap
 */

void
DOM_Document_destroyNamedNodeMap(DOM_Document *doc, DOM_NamedNodeMap *map, int free_nodes)
{
    DOM_Document_destroyNodeList(doc, map, free_nodes);
}
DOM_NamedNodeMap *
Document_createNamedNodeMap(DOM_Document *doc)
{
    return Document_createNodeList(doc);
}

DOM_Node *
DOM_NamedNodeMap_getNamedItem(const DOM_NamedNodeMap *map, const DOM_String *name)
{
	NodeEntry *e;
	unsigned short nodeType;

	if (map && name) {
		if (map->filter) {
			nodeType = map->filter;
			map = map->list;
			for (e = map->first; e != NULL; e = e->next) {
				if (e->node->nodeType == nodeType && strcoll(name, e->node->nodeName) == 0) {
					return e->node;
				}
			}
		} else {
			for (e = map->first; e != NULL; e = e->next) {
				if (strcoll(name, e->node->nodeName) == 0) {
					return e->node;
				}
			}
		}
	}

	return NULL;
}
DOM_Node *
DOM_NamedNodeMap_setNamedItem(DOM_NamedNodeMap *map, DOM_Node *arg)
{
	NodeEntry *e;

	if (map && arg) {
		if (map->filter) {
			DOM_Exception = DOM_NO_MODIFICATION_ALLOWED_ERR;
			PMNO(DOM_Exception);
			return NULL;
		}
		if (map->_ownerDocument != arg->ownerDocument) {
			DOM_Exception = DOM_WRONG_DOCUMENT_ERR;
			PMNO(DOM_Exception);
			return NULL;
		}
		if (arg->nodeType == DOM_ATTRIBUTE_NODE && arg->u.Attr.ownerElement &&
						arg->u.Attr.ownerElement != map->_ownerElement) {
			DOM_Exception = DOM_INUSE_ATTRIBUTE_ERR;
			PMNO(DOM_Exception);
			return NULL;
		}
		for (e = map->first; e != NULL && strcoll(arg->nodeName, e->node->nodeName); e = e->next) {
			;
		}
		if (e) {
			DOM_Node *tmp = e->node;
			e->node = arg;
			if (arg->nodeType == DOM_ATTRIBUTE_NODE) {
				arg->u.Attr.ownerElement = map->_ownerElement;
				tmp->u.Attr.ownerElement = NULL;
			}
			return tmp;
		}
		NodeList_append(map, arg);
	}

	return NULL;
}
DOM_Node *
DOM_NamedNodeMap_removeNamedItem(DOM_NamedNodeMap *map, const DOM_String *name)
{
	NodeEntry *e;
	DOM_Node *r = NULL;

	if (map && name) {
		if (map->filter) {
			DOM_Exception = DOM_NO_MODIFICATION_ALLOWED_ERR;
			PMNO(DOM_Exception);
			return NULL;
		}
		for (e = map->first; e != NULL; e = e->next) {
			if (strcoll(name, e->node->nodeName) == 0 &&
									NodeList_remove(map, e->node)) {
				r = e->node;
				free(e);
				if (r->nodeType == DOM_ATTRIBUTE_NODE) {
					r->u.Attr.ownerElement = NULL;
				}
				return r;
			}
		}
	}

	DOM_Exception = DOM_NOT_FOUND_ERR;
	PMNO(DOM_Exception);
	return NULL;
}
DOM_Node *
DOM_NamedNodeMap_item(const DOM_NamedNodeMap *map, int index)
{
	return DOM_NodeList_item(map, index);
}

