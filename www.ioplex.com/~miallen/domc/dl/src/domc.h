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

/* domc.h - document object model interface
 */

#ifndef DOMC_H
#define DOMC_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#if defined(__sparc__)
  #include <sys/inttypes.h>
#elif defined(_WIN32)
  typedef unsigned __int64 uint64_t;
#else
  #include <stdint.h>
#endif

/* Enable this for hashing and fast access to large child node lists */
#define FAST_NODELIST 1

#if FAST_NODELIST
#include <mba/hashmap.h>
#endif

/* DOM_String
 */

typedef char DOM_String;

/* DOM_TimeStamp - Introduced in DOM Level 2
 */

typedef uint64_t DOM_TimeStamp;

/* DOM_Exception
 */

extern int *_DOM_Exception(void);

#define DOM_Exception (*_DOM_Exception())

/* DOM_EventException - Introduced in DOM Level 2
 */

#define DOM_UNSPECIFIED_EVENT_TYPE_ERR  dom_codes[13].msgno

/* DOM_Node
 */

typedef struct DOM_Node DOM_Node;
typedef struct DOM_NodeList DOM_NodeList;
typedef struct DOM_NodeList DOM_NamedNodeMap;
typedef struct NodeEntry NodeEntry;

typedef DOM_Node DOM_Attr;
typedef DOM_Node DOM_Element;
typedef DOM_Node DOM_CharacterData;
typedef DOM_CharacterData DOM_Text;
typedef DOM_CharacterData DOM_Comment;
typedef DOM_Text DOM_CDATASection;
typedef DOM_Node DOM_DocumentType;
typedef DOM_Node DOM_Notation;
typedef DOM_Node DOM_Entity;
typedef DOM_Node DOM_EntityReference;
typedef DOM_Node DOM_ProcessingInstruction;
typedef DOM_Node DOM_DocumentFragment;
typedef DOM_Node DOM_Document;
typedef DOM_Document DOM_DocumentLS;
/* Introduced in DOM Level 2: */
typedef DOM_Node DOM_EventTarget;
typedef struct ListenerEntry ListenerEntry;
typedef DOM_Document DOM_DocumentEvent;
typedef DOM_Document DOM_AbstractView;
typedef DOM_Document DOM_DocumentView;

#define DOM_ELEMENT_NODE                1
#define DOM_ATTRIBUTE_NODE              2
#define DOM_TEXT_NODE                   3
#define DOM_CDATA_SECTION_NODE          4
#define DOM_ENTITY_REFERENCE_NODE       5
#define DOM_ENTITY_NODE                 6
#define DOM_PROCESSING_INSTRUCTION_NODE 7
#define DOM_COMMENT_NODE                8
#define DOM_DOCUMENT_NODE               9
#define DOM_DOCUMENT_TYPE_NODE          10
#define DOM_DOCUMENT_FRAGMENT_NODE      11
#define DOM_NOTATION_NODE               12

/* events forward references - Introduced in DOM Level 2
 */

typedef struct DOM_Event DOM_Event;
typedef struct DOM_Event DOM_UIEvent;
typedef DOM_UIEvent DOM_TextEvent;
typedef struct DOM_Event DOM_MutationEvent;

typedef void DOM_EventListener;
typedef void (*DOM_EventListener_handleEvent)(DOM_EventListener *this, DOM_Event *evt);

struct ListenerEntry {
	DOM_String *type;
	DOM_EventListener *listener;
	DOM_EventListener_handleEvent listener_fn;
	int useCapture;
};

struct DOM_Node {
    DOM_String *nodeName;
    DOM_String *nodeValue;
    unsigned short nodeType;
    DOM_Node *parentNode;
    DOM_NodeList *childNodes;
    DOM_Node *firstChild;
    DOM_Node *lastChild;
    DOM_Node *previousSibling;
    DOM_Node *nextSibling;
    DOM_NamedNodeMap *attributes;
    DOM_Document *ownerDocument;
	/* Custom Fields */
	unsigned int listeners_len;
	ListenerEntry **listeners;
	unsigned short subtreeModified;
    union {
        struct {
            DOM_DocumentType *doctype;
            DOM_Element *documentElement;
			DOM_DocumentView *document;
			DOM_AbstractView *defaultView;
			DOM_Node *commonParent;
			DOM_String *version;
			DOM_String *encoding;
			int standalone;
        } Document;
        struct {
            DOM_String *name;
            DOM_NamedNodeMap *entities;
            DOM_NamedNodeMap *notations;
			DOM_String *publicId;
			DOM_String *systemId;
			DOM_String *internalSubset;
        } DocumentType;
        struct {
            DOM_String *tagName;
        } Element;
        struct {
            DOM_String *name;
            int specified;
            DOM_String *value;
			DOM_Element *ownerElement;
        } Attr;
        struct {
            DOM_String *data;
            int length;
        } CharacterData;
        struct {
            DOM_String *publicId;
            DOM_String *systemId;
        } Notation;
       struct {
            DOM_String *publicId;
            DOM_String *systemId;
            DOM_String *notationName;
        } Entity;
        struct {
            DOM_String *target;
            DOM_String *data;
        } ProcessingInstruction;
    } u;
};

DOM_Node *DOM_Node_insertBefore(DOM_Node *node, DOM_Node *newChild, DOM_Node *refChild);
DOM_Node *DOM_Node_replaceChild(DOM_Node *node, DOM_Node *newChild, DOM_Node *oldChild);
DOM_Node *DOM_Node_removeChild(DOM_Node *node, DOM_Node *oldChild);
DOM_Node *DOM_Node_appendChild(DOM_Node *node, DOM_Node *newChild);
int DOM_Node_hasChildNodes(const DOM_Node *node);
DOM_Node *DOM_Node_cloneNode(DOM_Node *node, int deep);
DOM_String *DOM_Node_getNodeValue(DOM_Node *node);
void DOM_Node_setNodeValue(DOM_Node *node, DOM_String *value);

/* DOM_NodeList, DOM_NamedNodeMap
 */

struct NodeEntry {
	NodeEntry *prev;
	NodeEntry *next;
	DOM_Node *node;
};
struct DOM_NodeList {
    DOM_Document *_ownerDocument;
	DOM_Element *_ownerElement;
	int length;
	NodeEntry *first;
	NodeEntry *last;
	unsigned short filter;
	struct DOM_NodeList *list; /* Used for entities and notations */
#if FAST_NODELIST
	struct hashmap* _map;
#endif
};

DOM_Node *DOM_NodeList_item(const DOM_NodeList *nl, int index);

DOM_Node *DOM_NamedNodeMap_getNamedItem(const DOM_NamedNodeMap *map, const DOM_String *name);
DOM_Node *DOM_NamedNodeMap_setNamedItem(DOM_NamedNodeMap *map, DOM_Node *arg);
DOM_Node *DOM_NamedNodeMap_removeNamedItem(DOM_NamedNodeMap *map, const DOM_String *name);
DOM_Node *DOM_NamedNodeMap_item(const DOM_NamedNodeMap *map, int index);

/* DOM_Implementation
 */

int DOM_Implementation_hasFeature(DOM_String *feature, DOM_String *version);
DOM_DocumentType *DOM_Implementation_createDocumentType(DOM_String *qualifiedName,
										DOM_String *publicId, DOM_String *systemId);
DOM_Document *DOM_Implementation_createDocument(DOM_String *namespaceURI,
										DOM_String *qualifiedName, DOM_DocumentType *doctype);

/* DOM_Element
 */

DOM_String *DOM_Element_getAttribute(const DOM_Element *element, const DOM_String *name);
void DOM_Element_setAttribute(DOM_Element *element, const DOM_String *name,
										const DOM_String *value);
void DOM_Element_removeAttribute(DOM_Element *element, const DOM_String *name);
DOM_Attr *DOM_Element_getAttributeNode(const DOM_Element *element, const DOM_String *name);
DOM_Attr *DOM_Element_setAttributeNode(DOM_Element *element, DOM_Attr *newAttr);
DOM_Attr *DOM_Element_removeAttributeNode(DOM_Element *element, DOM_Attr *oldAttr);
DOM_NodeList *DOM_Element_getElementsByTagName(DOM_Element *element, const DOM_String *name);
void DOM_Element_normalize(DOM_Element *element);

/* DOM_CharacterData
 */

DOM_String *DOM_CharacterData_substringData(DOM_CharacterData *data,
										int offset, int count);
void DOM_CharacterData_appendData(DOM_CharacterData *data, const DOM_String *arg);
void DOM_CharacterData_insertData(DOM_CharacterData *data, int offset,
										const DOM_String *arg);
void DOM_CharacterData_deleteData(DOM_CharacterData *data, int offset, int count);
void DOM_CharacterData_replaceData(DOM_CharacterData *data, int offset, int count,
                                        const DOM_String *arg);
int DOM_CharacterData_getLength(DOM_CharacterData *data);

/* DOM_Text
 */

DOM_Text *DOM_Text_splitText(DOM_Text *text, int offset);

/* DOM_Document
 */

DOM_Element *DOM_Document_createElement(DOM_Document *doc, const DOM_String *tagName);
DOM_DocumentFragment *DOM_Document_createDocumentFragment(DOM_Document *doc);
DOM_Text *DOM_Document_createTextNode(DOM_Document *doc, const DOM_String *data);
DOM_Comment *DOM_Document_createComment(DOM_Document *doc, const DOM_String *data);
DOM_CDATASection *DOM_Document_createCDATASection(DOM_Document *doc, const DOM_String *data);
DOM_ProcessingInstruction *DOM_Document_createProcessingInstruction(DOM_Document *doc,
										const DOM_String *target, const DOM_String *data);
DOM_Attr *DOM_Document_createAttribute(DOM_Document *doc, const DOM_String *name);
DOM_EntityReference *DOM_Document_createEntityReference(DOM_Document *doc,
										const DOM_String *name);
DOM_NodeList *DOM_Document_getElementsByTagName(DOM_Document *doc, const DOM_String *tagname);

void DOM_Document_destroyNode(DOM_Document *doc, DOM_Node *node);
void DOM_Document_destroyNodeList(DOM_Document *doc, DOM_NodeList *nl, int free_nodes);

DOM_DocumentType *DOM_Document_getDoctype(DOM_Document *doc);
DOM_Element *DOM_Document_getDocumentElement(DOM_Document *doc);

/* DOM_DocumentLS - This does NOT resemble the Load/Save interfaces
 * described in the latest W3C drafts at all.
 */

int DOM_DocumentLS_load(DOM_DocumentLS *this, const char *uri);
int DOM_DocumentLS_fread(DOM_DocumentLS *this, FILE *stream);
int DOM_DocumentLS_save(DOM_DocumentLS *this, const char *uri, const DOM_Node *node);
int DOM_DocumentLS_fwrite(const DOM_DocumentLS *this, FILE *stream);

/* Events - Introduced in DOM Level 2
 * http://www.w3.org/TR/2000/REC-DOM-Level-2-Events-20001113/events.html
 */

/* DOM_Event - Introduced in DOM Level 2
 */

#define DOM_EVENT_CAPTURING_PHASE 1
#define DOM_EVENT_AT_TARGET       2
#define DOM_EVENT_BUBBLING_PHASE  3

struct DOM_Event {
	const DOM_String *type;
	DOM_EventTarget *target;
	DOM_EventTarget *currentTarget;
	unsigned short eventPhase;
	int bubbles;
	int cancelable;
	DOM_TimeStamp timeStamp;
/* custom -- do not touch */
	int _pd;
	int _sp;
	unsigned int _modifiers;
/* UIEvent members */
	DOM_AbstractView *view;
	long detail;
/* TextEvent members */
	DOM_String *outputString;
	unsigned long keyVal;
	unsigned long virtKeyVal;
	int visibleOutputGenerated;
	int numPad;
/* MutationEvent members */
	DOM_Node *relatedNode;
	DOM_String *prevValue;
	DOM_String *newValue;
	DOM_String *attrName;
	unsigned short attrChange;
};

void DOM_Event_stopPropagation(DOM_Event *evt);
void DOM_Event_preventDefault(DOM_Event *evt);
void DOM_Event_initEvent(DOM_Event *evt, const DOM_String *eventTypeArg,
								int canBubbleArg, int cancelableArg);

/* DOM_DocumentEvent - Introduced in DOM Level 2
 */

DOM_Event *DOM_DocumentEvent_createEvent(DOM_DocumentEvent *doc,
								const DOM_String *eventType);
void DOM_DocumentEvent_destroyEvent(DOM_DocumentEvent *doc, DOM_Event *evt);

/* DOM_UIEvent
 */

void DOM_UIEvent_initUIEvent(DOM_UIEvent *evt,
								const DOM_String *typeArg,
								int canBubbleArg,
								int cancelableArg,
								DOM_AbstractView *viewArg,
								long detailArg);

/* DOM_EventTarget - Introduced in DOM Level 2
 */

void DOM_EventTarget_addEventListener(DOM_EventTarget *target,
								const DOM_String *type,
								DOM_EventListener *listener,
								DOM_EventListener_handleEvent listener_fn,
								int useCapture);
void DOM_EventTarget_removeEventListener(DOM_EventTarget *target,
								const DOM_String *type,
								DOM_EventListener *listener,
								DOM_EventListener_handleEvent listener_fn,
								int useCapture);
int DOM_EventTarget_dispatchEvent(DOM_EventTarget *target, DOM_Event *evt);

/* DOM_TextEvent - Introduced in DOM Level 3
 * http://www.w3.org/TR/2002/WD-DOM-Level-3-Events-20020208/events.html
 */

#define DOM_VK_UNDEFINED     0x0
#define DOM_VK_RIGHT_ALT     0x01
#define DOM_VK_LEFT_ALT      0x02
#define DOM_VK_LEFT_CONTROL  0x03
#define DOM_VK_RIGHT_CONTROL 0x04
#define DOM_VK_LEFT_SHIFT    0x05
#define DOM_VK_RIGHT_SHIFT   0x06
#define DOM_VK_LEFT_META     0x07
#define DOM_VK_RIGHT_META    0x08
#define DOM_VK_CAPS_LOCK     0x09
#define DOM_VK_DELETE        0x0A
#define DOM_VK_END           0x0B
#define DOM_VK_ENTER         0x0C
#define DOM_VK_ESCAPE        0x0D
#define DOM_VK_HOME          0x0E
#define DOM_VK_INSERT        0x0F
#define DOM_VK_NUM_LOCK      0x10
#define DOM_VK_PAUSE         0x11
#define DOM_VK_PRINTSCREEN   0x12
#define DOM_VK_SCROLL_LOCK   0x13
#define DOM_VK_LEFT          0x14
#define DOM_VK_RIGHT         0x15
#define DOM_VK_UP            0x16
#define DOM_VK_DOWN          0x17
#define DOM_VK_PAGE_DOWN     0x18
#define DOM_VK_PAGE_UP       0x19
#define DOM_VK_F1            0x1A
#define DOM_VK_F2            0x1B
#define DOM_VK_F3            0x1C
#define DOM_VK_F4            0x1D
#define DOM_VK_F5            0x1E
#define DOM_VK_F6            0x1F
#define DOM_VK_F7            0x20
#define DOM_VK_F8            0x21
#define DOM_VK_F9            0x22
#define DOM_VK_F10           0x23
#define DOM_VK_F11           0x24
#define DOM_VK_F12           0x25
#define DOM_VK_F13           0x26
#define DOM_VK_F14           0x27
#define DOM_VK_F15           0x28
#define DOM_VK_F16           0x29
#define DOM_VK_F17           0x2A
#define DOM_VK_F18           0x2B
#define DOM_VK_F19           0x2C
#define DOM_VK_F20           0x2D
#define DOM_VK_F21           0x2E
#define DOM_VK_F22           0x2F
#define DOM_VK_F23           0x30
#define DOM_VK_F24           0x31

int DOM_TextEvent_checkModifier(DOM_TextEvent *evt, unsigned int modifier);
void DOM_TextEvent_initTextEvent(DOM_TextEvent *evt, const DOM_String *typeArg,
										int canBubbleArg, int cancelableArg,
										DOM_AbstractView *viewArg, long detailArg,
										DOM_String *outputStringArg,
										unsigned int keyValArg,
										unsigned int virtKeyValArg,
										int visibleOutputGeneratedArg,
										int numPadArg);
void DOM_TextEvent_initModifier(DOM_TextEvent *evt, unsigned int modifier, int value);

/* Useful X KeySym to keyVal and virtKeyVal conversion functions
 * Add these .o's to the OBJS line in the Makefile to compile
 */

unsigned int keysym2ucs(unsigned short keysym);
unsigned int keysym2domvk(unsigned short keysym);

/* MutationEvent
 */

#define DOM_MUTATION_EVENT_MODIFICATION 1
#define DOM_MUTATION_EVENT_ADDITION     2
#define DOM_MUTATION_EVENT_REMOVAL      3

void DOM_MutationEvent_initMutationEvent(DOM_MutationEvent *evt,
										DOM_String *typeArg,
										int canBubbleArg,
										int cancelableArg,
										DOM_Node *relatedNodeArg,
										DOM_String *prevValueArg,
										DOM_String *newValueArg,
										DOM_String *attrNameArg,
										unsigned short attrChangeArg);
void DOM_MutationEvent_processSubtreeModified(DOM_EventTarget *subtree);

/* Temporary Functions
 */

void DOM_Node_printNode(DOM_Node *node);
void DOM_Node_printNode2(DOM_Node *node);

#endif /* DOMC_H */

