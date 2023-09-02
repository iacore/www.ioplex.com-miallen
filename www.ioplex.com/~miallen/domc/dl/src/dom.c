/* domc document object model library in c
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

/* dom.c - document object model interface
 */

#include <string.h>
#include <wchar.h>
#include <limits.h>
#include "defines.h"
#include <mba/msgno.h>
#include "mbs.h"
#include "domc.h"
#include "dom.h"

#if HAVE_STRDUP < 1

static char *
strdup(const char *s)
{
	return s ? strcpy(malloc(strlen(s) + 1), s) : NULL;
}

#endif

/* Forward references for node.c
 */

DOM_Node *Document_createNode(DOM_Document *doc, unsigned short nodeType);
NodeEntry *NodeList_remove(DOM_NodeList *nl, DOM_Node *oldChild);
NodeEntry *NodeList_append(DOM_NodeList *nl, DOM_Node *newChild);
DOM_NodeList *Document_createNodeList(DOM_Document *doc);
DOM_NamedNodeMap *Document_createNamedNodeMap(DOM_Document *doc);

/* Forward references for events.c
 */

void updateCommonParent(DOM_Node *node);

/* DOM_Exception
 */

struct msgno_entry dom_codes[] = {
	{ 1, "The index specified was out of range" },
	{ 0, "The text size is out of range" },
	{ 0, "The request violated tree hierarchy constraints" },
	{ 0, "The document context is invalid" },
	{ 0, "An inappropriate character was encountered" },
	{ 0, "The node does not support the addition of data" },
	{ 0, "No modification allowed" },
	{ 0, "The specified node was not found" },
	{ 0, "The requested operation is not supported" },
	{ 0, "The attribute is being used elsewhere" },
	{ 0, "An XML parser error occured" },
	{ 0, "Failed to create DOM object" },
	{ 0, "Character encoding error" },
	{ 0, "The event type was not specified by initialization" },
	{ 0, "A filtered list cannot be modified" },
    { 0, NULL }
};

int _exception = 0;

int *
_DOM_Exception(void)
{
	return &_exception;
}

unsigned short child_matrix[] = {
	0x00dd, /* DOM_ELEMENT_NODE                1 */
	0x0014, /* DOM_ATTRIBUTE_NODE              2 */
	0x0000, /* DOM_TEXT_NODE                   3 */
	0x0000, /* DOM_CDATA_SECTION_NODE          4 */
	0x00dd, /* DOM_ENTITY_REFERENCE_NODE       5 */
	0x00dd, /* DOM_ENTITY_NODE                 6 */
	0x0000, /* DOM_PROCESSING_INSTRUCTION_NODE 7 */
	0x0000, /* DOM_COMMENT_NODE                8 */
	0x02c1, /* DOM_DOCUMENT_NODE               9 */
	0x0820, /* DOM_DOCUMENT_TYPE_NODE          10 */
	0x00dd, /* DOM_DOCUMENT_FRAGMENT_NODE      11 */
	0x0000  /* DOM_NOTATION_NODE               12 */
};

const char *node_names[] = {
	"No such node type",
	"DOM_ELEMENT_NODE",
	"DOM_ATTRIBUTE_NODE",
	"DOM_TEXT_NODE",
	"DOM_CDATA_SECTION_NODE",
	"DOM_ENTITY_REFERENCE_NODE",
	"DOM_ENTITY_NODE",
	"DOM_PROCESSING_INSTRUCTION_NODE",
	"DOM_COMMENT_NODE",
	"DOM_DOCUMENT_NODE",
	"DOM_DOCUMENT_TYPE_NODE",
	"DOM_DOCUMENT_FRAGMENT_NODE",
	"DOM_NOTATION_NODE"
};

/* DOM_Implementation and DOM_ImplementationLS
 */

int
DOM_Implementation_hasFeature(DOM_String *feature, DOM_String *version)
{
	feature = NULL; version = NULL;
	return 0;
}
DOM_DocumentType *
DOM_Implementation_createDocumentType(DOM_String *qualifiedName,
						DOM_String *publicId, DOM_String *systemId)
{
    DOM_DocumentType *doctype;
	DOM_NamedNodeMap *entities;
	DOM_NamedNodeMap *notations;

    if ((doctype = Document_createNode(NULL, DOM_DOCUMENT_TYPE_NODE)) == NULL) {
		AMSG("");
		return NULL;
	}
	if ((doctype->nodeName = doctype->u.DocumentType.name = strdup(qualifiedName)) == NULL ||
			(publicId && ((doctype->u.DocumentType.publicId = strdup(publicId))) == NULL) ||
			(systemId && ((doctype->u.DocumentType.systemId = strdup(systemId))) == NULL)) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		DOM_Document_destroyNode(NULL, doctype);
		return NULL;
	}
	if ((entities = Document_createNamedNodeMap(NULL)) == NULL ||
			(notations = Document_createNamedNodeMap(NULL)) == NULL) {
		AMNO(DOM_CREATE_FAILED);
		DOM_Document_destroyNode(NULL, doctype);
		return NULL;
	}
	entities->filter = DOM_ENTITY_NODE;
	notations->filter = DOM_NOTATION_NODE;
	entities->list = notations->list = doctype->childNodes;
	doctype->u.DocumentType.entities = entities;
	doctype->u.DocumentType.notations = notations;

    return doctype;
}
DOM_Document *
DOM_Implementation_createDocument(DOM_String *namespaceURI,
						DOM_String *qualifiedName, DOM_DocumentType *doctype)
{
    DOM_Document *doc;
    DOM_Element *el;

	namespaceURI = NULL; 

	msgno_add_codes(dom_codes);

    if ((doc = Document_createNode(NULL, DOM_DOCUMENT_NODE)) == NULL) {
		AMSG("");
		return NULL;
	}
	doc->nodeName = "#document";
	if (doctype) {
		DOM_Node_appendChild(doc, doctype);
	}

    if (qualifiedName && *qualifiedName) {
        if ((el = DOM_Document_createElement(doc, qualifiedName)) == NULL) {
            AMSG("");
            DOM_Document_destroyNode(doc, doc);
            return NULL;
        }
        DOM_Node_appendChild(doc, el);
    }

    return doc;
}

DOM_String *
DOM_Element_getAttribute(const DOM_Element *element, const DOM_String *name)
{
	DOM_Node *node;
	DOM_String *r = NULL;

	if (element && name && element->attributes) {
		if ((node = DOM_NamedNodeMap_getNamedItem(element->attributes, name))) {
			if ((r = strdup(node->nodeValue)) == NULL) {
				AMSG("");
				return NULL;
			}
		} else if ((r = strdup("")) == NULL) {
			PMNO(errno);
			return NULL;
		}
	}

	return r;
}
void
DOM_Element_setAttribute(DOM_Element *element,
						const DOM_String *name, const DOM_String *value)
{
	DOM_Attr *attr;
	DOM_String *prevValue;
	unsigned short attrChange;
	DOM_MutationEvent evt;

	if (element == NULL || name == NULL ||
			value == NULL || element->attributes == NULL) {
		return;
	}

	attr = DOM_NamedNodeMap_getNamedItem(element->attributes, name);
	if (attr) {
		prevValue = attr->nodeValue;
		attrChange = DOM_MUTATION_EVENT_MODIFICATION;
		if ((attr->nodeValue = attr->u.Attr.value = strdup(value)) == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			DOM_Document_destroyNode(attr->ownerDocument, attr);
			return;
		}
	} else {
		prevValue = NULL;
		attrChange = DOM_MUTATION_EVENT_ADDITION;
		if ((attr = DOM_Document_createAttribute(element->ownerDocument, name)) == NULL) {
			AMNO(DOM_CREATE_FAILED);
			return;
		}
		free(attr->nodeValue);
		if ((attr->nodeValue = attr->u.Attr.value = strdup(value)) == NULL) {
			DOM_Exception = errno;
			DOM_Document_destroyNode(attr->ownerDocument, attr);
			return;
		}
		DOM_NamedNodeMap_setNamedItem(element->attributes, attr);
	}

	DOM_MutationEvent_initMutationEvent(&evt, "DOMAttrModified", 1, 0,
				attr, prevValue, attr->nodeValue, attr->nodeName, attrChange);
/*MSG("attr=%p,prevValue=[%s],newValue=[%s],attrName=%s", attr, prevValue, attr->nodeValue, attr->nodeName);
 */
	DOM_EventTarget_dispatchEvent(element, &evt);

	updateCommonParent(element->parentNode);

	free(prevValue);
}
void
DOM_Element_removeAttribute(DOM_Element *element, const DOM_String *name)
{
	DOM_Attr *attr;
	DOM_MutationEvent evt;

	if (element == NULL || name == NULL) {
		return;
	}

	attr = DOM_NamedNodeMap_removeNamedItem(element->attributes, name);
    
    /* removeAttribute doesn't throw exceptions on NOT_FOUND_ERR */
    if (DOM_Exception == DOM_NOT_FOUND_ERR)
        DOM_Exception = 0;

    if (attr) {
  	    DOM_MutationEvent_initMutationEvent(&evt, "DOMAttrModified", 1, 0,
		      attr, attr->nodeValue, NULL, attr->nodeName, DOM_MUTATION_EVENT_REMOVAL);
	    DOM_EventTarget_dispatchEvent(element, &evt);

	    updateCommonParent(element->parentNode);

        DOM_Document_destroyNode(attr->ownerDocument, attr);
	}
}
DOM_Attr *
DOM_Element_getAttributeNode(const DOM_Element *element, const DOM_String *name)
{
	if (element && name) {
		return DOM_NamedNodeMap_getNamedItem(element->attributes, name);
	}
	return NULL;
}
DOM_Attr *
DOM_Element_setAttributeNode(DOM_Element *element, DOM_Attr *newAttr)
{
	DOM_Node *attr;
	DOM_MutationEvent evt;

	if (element == NULL || newAttr == NULL) {
		return NULL;
	}
	if (element->ownerDocument != newAttr->ownerDocument) {
		DOM_Exception = DOM_WRONG_DOCUMENT_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	attr = DOM_NamedNodeMap_setNamedItem(element->attributes, newAttr);

	if (attr) {
		DOM_MutationEvent_initMutationEvent(&evt, "DOMAttrModified", 1, 0,
			attr, attr->nodeValue, NULL, attr->nodeName, DOM_MUTATION_EVENT_REMOVAL);
		DOM_EventTarget_dispatchEvent(element, &evt);
	}

	DOM_MutationEvent_initMutationEvent(&evt, "DOMAttrModified", 1, 0,
		newAttr, NULL, newAttr->nodeValue, newAttr->nodeName, DOM_MUTATION_EVENT_ADDITION);
	DOM_EventTarget_dispatchEvent(element, &evt);

	updateCommonParent(element->parentNode);

	return attr;
}
DOM_Attr *
DOM_Element_removeAttributeNode(DOM_Element *element, DOM_Attr *oldAttr)
{
	DOM_MutationEvent evt;

	if (element == NULL || oldAttr == NULL ||
				NodeList_remove(element->attributes, oldAttr) == NULL) {
		DOM_Exception = DOM_NOT_FOUND_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	DOM_MutationEvent_initMutationEvent(&evt, "DOMAttrModified", 1, 0,
		oldAttr, oldAttr->nodeValue, NULL, oldAttr->nodeName, DOM_MUTATION_EVENT_REMOVAL);
	DOM_EventTarget_dispatchEvent(element, &evt);

	updateCommonParent(element->parentNode);

	return oldAttr;
}

static void
getElementsPreorder(DOM_NodeList *list, DOM_Node *node, const DOM_String *tagname)
{
	DOM_Node *n;

	if (list && node && node->nodeType == DOM_ELEMENT_NODE && tagname) {
		if ((tagname[0] == '*' && tagname[1] == '\0') ||
					strcoll(tagname, node->nodeName) == 0) {
			NodeList_append(list, node);
		}
		for (n = node->firstChild; n != NULL; n = n->nextSibling) {
			getElementsPreorder(list, n, tagname);
		}
	}
}

DOM_NodeList *
DOM_Element_getElementsByTagName(DOM_Element *element, const DOM_String *name)
{
	DOM_NodeList *list;
	DOM_Node *n;

	if (element && element->nodeType == DOM_ELEMENT_NODE && name &&
				(list = Document_createNodeList(element->ownerDocument))) {
		for (n = element->firstChild; n != NULL; n = n->nextSibling) {
			getElementsPreorder(list, n, name);
		}
		return list;
	}

    return NULL;
}
void
DOM_Element_normalize(DOM_Element *element)
{
	DOM_Node *node;
	DOM_Text *last = NULL;

	if (element) {
		for (node = element->firstChild; node != NULL; node = node->nextSibling) {
			if (node->nodeType == DOM_TEXT_NODE) {
				if (last) {
					DOM_CharacterData_insertData(node, 0, last->nodeValue);
					DOM_Node_removeChild(element, last);
					DOM_Document_destroyNode(last->ownerDocument, last);
				}
				last = node;
			} else {
				last = NULL;
				DOM_Element_normalize(node);
			}
			if (DOM_Exception) {
				AMSG("");
				return;
			}
		}
	}
}

DOM_String *
DOM_CharacterData_substringData(DOM_CharacterData *data,
                                            int offset, int count)
{
    DOM_String *sub;
	int dlen;

	if (data == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (offset < 0 || offset > (dlen = data->u.CharacterData.length) || count < 0) {
		DOM_Exception = DOM_INDEX_SIZE_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	if (count > (dlen - offset)) {
		count = dlen - offset;
	}
	if ((sub = mbsoff(data->nodeValue, offset)) == NULL ||
			(sub = mbsndup(sub, -1, count)) == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		return NULL;
	}
	return sub;
}
void
DOM_CharacterData_appendData(DOM_CharacterData *data, const DOM_String *arg)
{
	DOM_String *str, *prevValue;
	size_t dsize, asize;
	DOM_MutationEvent evt;

	if (data == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (arg) {
		dsize = mbssize(data->nodeValue);
		asize = mbssize(arg);
		if ((str = malloc(dsize + asize + 1)) == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			return;
		}

		memcpy(str, data->nodeValue, dsize);
		memcpy(str + dsize, arg, asize);
		str[dsize + asize] = '\0';

		prevValue = data->nodeValue;
		data->nodeValue = data->u.CharacterData.data = str;
		data->u.CharacterData.length += mbslen(arg);

		DOM_MutationEvent_initMutationEvent(&evt, "DOMCharacterDataModified", 1, 0,
				NULL, prevValue, data->nodeValue, NULL, 0);
		DOM_EventTarget_dispatchEvent(data, &evt);

		updateCommonParent(data->parentNode);

		free(prevValue);
	}
}
void
DOM_CharacterData_insertData(DOM_CharacterData *data,
                                       int offset, const DOM_String *arg)
{
	DOM_String *str, *prevValue;
	size_t dsize, asize, o;
	DOM_MutationEvent evt;

	if (data == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (offset < 0 || offset > data->u.CharacterData.length) {
		DOM_Exception = DOM_INDEX_SIZE_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (arg) {
		dsize = mbssize(data->nodeValue);
		asize = mbssize(arg);
		if ((str = malloc(dsize + asize + 1)) == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			return;
		}

		o = mbsoff(data->nodeValue, offset) - data->nodeValue;
		memcpy(str, data->nodeValue, o);
		memcpy(str + o, arg, asize);
		memcpy(str + o + asize, data->nodeValue + o, dsize - o);
		str[dsize + asize] = '\0';

		prevValue = data->nodeValue;
		data->nodeValue = data->u.CharacterData.data = str;
		data->u.CharacterData.length += mbslen(arg);

		DOM_MutationEvent_initMutationEvent(&evt, "DOMCharacterDataModified", 1, 0,
				NULL, prevValue, data->nodeValue, NULL, 0);
		DOM_EventTarget_dispatchEvent(data, &evt);

		updateCommonParent(data->parentNode);

		free(prevValue);
	}
}
void
DOM_CharacterData_deleteData(DOM_CharacterData *data,
                                       int offset, int count)
{
	DOM_String *p1, *p2, *str, *prevValue;
	int p1size, p2size, dlen;
	DOM_MutationEvent evt;

	if (data == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (offset < 0 || offset > (dlen = data->u.CharacterData.length)) {
		DOM_Exception = DOM_INDEX_SIZE_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (count < 0 || (offset + count) > dlen) {
		count = dlen - offset;
	}

	p1 = mbsoff(data->nodeValue, offset);
	p1size = p1 - data->nodeValue;
	p2 = mbsoff(p1, count);
	p2size = strlen(p2);

	if ((str = malloc(p1size + p2size + 1)) == NULL) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		return;
	}
	memcpy(str, data->nodeValue, p1size);
	memcpy(str + p1size, p2, p2size);
	str[p1size + p2size] = '\0';

	prevValue = data->nodeValue;
	data->nodeValue = data->u.CharacterData.data = str;
	data->u.CharacterData.length = dlen - count;

	DOM_MutationEvent_initMutationEvent(&evt, "DOMCharacterDataModified", 1, 0,
				NULL, prevValue, data->nodeValue, NULL, 0);
	DOM_EventTarget_dispatchEvent(data, &evt);

	updateCommonParent(data->parentNode);

	free(prevValue);
}
void
DOM_CharacterData_replaceData(DOM_CharacterData *data, int offset, int count,
                                        const DOM_String *arg)
{
	DOM_CharacterData_deleteData(data, offset, count);
	DOM_CharacterData_insertData(data, offset, arg);
}
int
DOM_CharacterData_getLength(DOM_CharacterData *data)
{
	return data ? data->u.CharacterData.length : 0;
}

DOM_Text *
DOM_Text_splitText(DOM_Text *text, int offset)
{
	DOM_Text *node;

	if (text == NULL || text->parentNode == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}

	if (offset < 0 || offset > text->u.CharacterData.length) {
		DOM_Exception = DOM_INDEX_SIZE_ERR;
		PMNO(DOM_Exception);
		return NULL;
	}
	node = DOM_Document_createTextNode(text->ownerDocument,
										mbsoff(text->nodeValue, offset));
	if (node == NULL) {
		AMNO(DOM_CREATE_FAILED);
		return NULL;
	}
	DOM_CharacterData_deleteData(text, offset, -1);
	DOM_Node_insertBefore(text->parentNode, node, text->nextSibling);

	return node;
}


DOM_Element *
DOM_Document_createElement(DOM_Document *doc, const DOM_String *tagName)
{
    DOM_Element *element;

    element = Document_createNode(doc, DOM_ELEMENT_NODE);
    if (element) {
		if ((element->nodeName = element->u.Element.tagName = strdup(tagName)) == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			DOM_Document_destroyNode(doc, element);
			return NULL;
		}
		if ((element->attributes = Document_createNamedNodeMap(doc)) == NULL) {
			AMNO(DOM_CREATE_FAILED);
			DOM_Document_destroyNode(doc, element);
			return NULL;
		}
		element->attributes->_ownerElement = element; /* for Attr.ownerElement */
    }

    return element;
}
DOM_DocumentFragment *
DOM_Document_createDocumentFragment(DOM_Document *doc)
{
    DOM_DocumentFragment *frag;

    frag = Document_createNode(doc, DOM_DOCUMENT_FRAGMENT_NODE);
    if (frag) {
        frag->nodeName = "#document-fragment";
    }

    return frag;
}
DOM_Text *
DOM_Document_createTextNode(DOM_Document *doc, const DOM_String *data)
{
    DOM_Text *text;

    text = Document_createNode(doc, DOM_TEXT_NODE);
    if (text) {
        text->nodeName = "#text";
        text->nodeValue = text->u.CharacterData.data = strdup(data);
        if (text->nodeValue == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
            DOM_Document_destroyNode(doc, text);
            return NULL;
        }
        text->u.CharacterData.length = mbslen(data);
    }

    return text;
}
DOM_Comment *
DOM_Document_createComment(DOM_Document *doc, const DOM_String *data)
{
    DOM_Comment *comment;

    comment = Document_createNode(doc, DOM_COMMENT_NODE);
    if (comment) {
        comment->nodeName = "#comment";
        comment->nodeValue = comment->u.CharacterData.data = strdup(data);
        if (comment->nodeValue == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
            DOM_Document_destroyNode(doc, comment);
            return NULL;
        }
        comment->u.CharacterData.length = mbslen(data);
    }

    return comment;
}
DOM_CDATASection *
DOM_Document_createCDATASection(DOM_Document *doc, const DOM_String *data)
{
    DOM_CDATASection *cdata;

    cdata = Document_createNode(doc, DOM_CDATA_SECTION_NODE);
    if (cdata) {
        cdata->nodeName = "#cdata-section";
        cdata->nodeValue = cdata->u.CharacterData.data = strdup(data);
        if (cdata->u.CharacterData.data == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
            DOM_Document_destroyNode(doc, cdata);
            return NULL;
        }
        cdata->u.CharacterData.length = mbslen(data);
    }

    return cdata;
}
DOM_ProcessingInstruction *
DOM_Document_createProcessingInstruction(DOM_Document *doc,
                                         const DOM_String *target, const DOM_String *data)
{
    DOM_ProcessingInstruction *pi;

    pi = Document_createNode(doc, DOM_PROCESSING_INSTRUCTION_NODE);
    if (pi) {
        pi->nodeName = pi->u.ProcessingInstruction.target = strdup(target);
        pi->nodeValue = pi->u.ProcessingInstruction.data = strdup(data);
        if (pi->nodeName == NULL || pi->nodeValue == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
            DOM_Document_destroyNode(doc, pi);
            return NULL;
        }
    }

    return pi;
}
DOM_Attr *
DOM_Document_createAttribute(DOM_Document *doc, const DOM_String *name)
{
    DOM_Attr *attr;

    attr = Document_createNode(doc, DOM_ATTRIBUTE_NODE);
    if (attr) {
        attr->nodeName = attr->u.Attr.name = strdup(name);
        attr->nodeValue = attr->u.Attr.value = strdup("");
		attr->u.Attr.specified = 1;
        if (attr->nodeName == NULL || attr->nodeValue == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
            DOM_Document_destroyNode(doc, attr);
            return NULL;
        }
    }

    return attr;
}
DOM_EntityReference *
DOM_Document_createEntityReference(DOM_Document *doc, const DOM_String *name)
{
    DOM_EntityReference *eref;

    eref = Document_createNode(doc, DOM_ENTITY_REFERENCE_NODE);
	if (eref && (eref->nodeName = strdup(name)) == NULL) {
		DOM_Document_destroyNode(doc, eref);
		return NULL;
	}

    return eref;
}

DOM_NodeList *
DOM_Document_getElementsByTagName(DOM_Document *doc, const DOM_String *tagname)
{
	DOM_NodeList *list;

	if (doc && doc->nodeType == DOM_DOCUMENT_NODE &&
				tagname && (list = Document_createNodeList(doc))) {
		getElementsPreorder(list, doc->u.Document.documentElement, tagname);
		return list;
	}

    return NULL;
}
DOM_DocumentType *
DOM_Document_getDoctype(DOM_Document *doc)
{
	return doc ? doc->u.Document.doctype : NULL;
}
DOM_Element *
DOM_Document_getDocumentElement(DOM_Document *doc)
{
	return doc ? doc->u.Document.documentElement : NULL;
}

/* Temporary functions */

void
printNode(DOM_Node *node, int indent)
{
	DOM_Node *n;
	int i;

    if (node == NULL) {
        printf("node was null\n");
        return;
    }

	for (i = 0; i < indent; i++) {
		printf("    ");
	}

	printf("%s: %s=%s\n", node_names[node->nodeType], node->nodeName, node->nodeValue);

	if (node->nodeType == DOM_ELEMENT_NODE && node->attributes->length) {
		printf("    ");
		n = DOM_NamedNodeMap_item(node->attributes, 0);
		printf("%s=%s", n->nodeName, n->nodeValue);
		for (i = 1; i < node->attributes->length; i++) {
			n = DOM_NamedNodeMap_item(node->attributes, i);
			printf(",%s=%s", n->nodeName, n->nodeValue);
		}
		printf("\n");
		for (i = 0; i < indent; i++) {
			printf("    ");
		}
	}
	for (n = node->firstChild; n != NULL; n = n->nextSibling) {
		printNode(n, indent + 1);
	}
}
void
DOM_Node_printNode2(DOM_Node *node)
{
	printf("\n");
	printNode(node, 0);
}
void
DOM_Node_printNode(DOM_Node *node)
{
    if (node == NULL) {
        printf("node was null\n");
        return;
    }

    printf("\nnodeName=%s,nodeValue=%s,", node->nodeName, node->nodeValue);
	printf("\n\ttype=%u", node->nodeType);
    printf(",parentNode->nodeName=%s,firstChild->nodeName=%s", (node->parentNode == NULL ? "(null)" : node->parentNode->nodeName), (node->firstChild == NULL ? "(null)" : node->firstChild->nodeName));
	printf(",lastChild->nodeName=%s,\n\tchildNodes->length=%u", (node->lastChild == NULL ? "(null)" : node->lastChild->nodeName), (node->childNodes == NULL ? 0 : node->childNodes->length));
    printf(",previousSibling->nodeName=%s,nextSibling->nodeName=%s,attributes->length=%u\n", (node->previousSibling == NULL ? "(null)" : node->previousSibling->nodeName), (node->nextSibling == NULL ? "(null)": node->nextSibling->nodeName), (node->attributes == NULL ? 0 : node->attributes->length));
    fflush(stdout);
}

