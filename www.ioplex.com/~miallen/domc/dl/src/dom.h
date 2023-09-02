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

/* dom.h - internal interfaces
 */

#ifndef DOM_H
#define DOM_H

#include <mba/msgno.h>

extern struct msgno_entry dom_codes[];

#define DOM_INDEX_SIZE_ERR              dom_codes[0].msgno
#define DOM_DOMSTRING_SIZE_ERR          dom_codes[1].msgno
#define DOM_HIERARCHY_REQUEST_ERR       dom_codes[2].msgno
#define DOM_WRONG_DOCUMENT_ERR          dom_codes[3].msgno
#define DOM_INVALID_CHARACTER_ERR       dom_codes[4].msgno
#define DOM_NO_DATA_ALLOWED_ERR         dom_codes[5].msgno
#define DOM_NO_MODIFICATION_ALLOWED_ERR dom_codes[6].msgno
#define DOM_NOT_FOUND_ERR               dom_codes[7].msgno
#define DOM_NOT_SUPPORTED_ERR           dom_codes[8].msgno
#define DOM_INUSE_ATTRIBUTE_ERR         dom_codes[9].msgno
#define DOM_XML_PARSER_ERR              dom_codes[10].msgno
#define DOM_CREATE_FAILED               dom_codes[11].msgno
#define DOM_CHARACTER_ENC_ERR           dom_codes[12].msgno
#define DOM_UNSPECIFIED_EVENT_TYPE_ERR  dom_codes[13].msgno
#define DOM_FILTERED_LIST_ERR           dom_codes[14].msgno

#define CANNOT_HAVE_CHILD_OF(p, c) !(child_matrix[(p)->nodeType - 1] & \
					(1 << ((c)->nodeType - 1)))
#define MODIFYING_DOC_ELEM(p, c) ((p)->nodeType == DOM_DOCUMENT_NODE && \
					(c)->nodeType == DOM_ELEMENT_NODE)
#define MODIFYING_DOCTYPE_ELEM(p, c) ((p)->nodeType == DOM_DOCUMENT_NODE && \
					(c)->nodeType == DOM_DOCUMENT_TYPE_NODE)
#define INVALID_HIER_REQ(p, c) (CANNOT_HAVE_CHILD_OF(p, c) || \
					(MODIFYING_DOC_ELEM(p, c) && (p)->u.Document.documentElement))

extern unsigned short child_matrix[];

DOM_NodeList *Document_createNodeList(DOM_Document *doc);
DOM_Node *NodeList_itemFiltered(const DOM_NodeList *list, int index, unsigned short nodeType);
NodeEntry *NodeList_insert(DOM_NodeList *nl, DOM_Node *newChild, DOM_Node *refChild);
NodeEntry *NodeList_replace(DOM_NodeList *nl, DOM_Node *newChild, DOM_Node *oldChild);
NodeEntry *NodeList_remove(DOM_NodeList *nl, DOM_Node *oldChild);
NodeEntry *NodeList_append(DOM_NodeList *nl, DOM_Node *newChild);
int NodeList_exists(DOM_NodeList *nl, DOM_Node *child);

DOM_NamedNodeMap *Document_createNamedNodeMap(DOM_Document *doc);

#endif /* DOM_H */

