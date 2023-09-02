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

/* expatls.c - DOM_DocumentLS
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "defines.h"
#include <mba/msgno.h>
#include <mba/stack.h>
#include "mbs.h"
#include "domc.h"
#include "dom.h"

#if HAVE_LANGINFO > 0
#include <langinfo.h>
#endif

#if HAVE_ENCDEC > 0
#include <encdec.h>
#elif HAVE_STRNLEN < 1

static size_t
strnlen(const char *s, size_t maxlen)
{
        size_t len;
        for (len = 0; *s && len < maxlen; s++, len++);
        return len;
}

#else

size_t strnlen(const char *s, size_t maxlen);

#endif

#if HAVE_EXPAT > 130
#include <expat.h>
#elif HAVE_EXPAT > 0
#include <xmlparse.h>
#endif

/* Forward references for node.c
 */

DOM_Node *Document_createNode(DOM_Document *doc, unsigned short nodeType);

static int
fputds(const DOM_String *s, FILE *stream)
{
	return fputs(s, stream);
}

#if HAVE_EXPAT > 0

struct user_data {
	DOM_String *buf;
	size_t siz;
	struct stack *stk;
	int cdata;
};

static size_t
utf8tods(const char *src, size_t sn, struct user_data *ud)
{
	size_t n;

#if HAVE_ENCDEC > 0
	char *s;

	s = (char *)src;
	if ((n = dec_mbsncpy(&s, sn, NULL, -1, -1, "UTF-8")) == (size_t)-1) {
		DOM_Exception = DOM_CHARACTER_ENC_ERR;
		PMNO(DOM_Exception);
		return -1;
	}
#else
	n = strnlen(src, sn) + 1;
#endif
	if (n > ud->siz) {
		ud->siz = n > (2 * ud->siz) ? n : (2 * ud->siz);
		if ((ud->buf = realloc(ud->buf, ud->siz)) == NULL) {
			DOM_Exception = errno;
			PMNO(DOM_Exception);
			return -1;
		}
	}
#if HAVE_ENCDEC > 0
	if ((n = dec_mbsncpy(&s, sn, ud->buf, ud->siz, -1, "UTF-8")) == (size_t)-1) {
		DOM_Exception = DOM_CHARACTER_ENC_ERR;
		PMNO(DOM_Exception);
		return -1;
	}
#else
	strncpy(ud->buf, src, n);
	ud->buf[n - 1] = '\0';
#endif
	return n;
}

static void
xmldecl_fn(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone)
{
	struct user_data *ud = userData;
	DOM_Document *doc;
	size_t n;

	if (DOM_Exception) {
		return;
	}

	doc = stack_peek(ud->stk);
	if (doc == NULL) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	doc->u.Document.version = doc->u.Document.encoding = NULL;
	if (version && ((n = utf8tods(version, 16, ud)) == (size_t)-1 ||
				(doc->u.Document.version = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		return;
	}
	if (encoding && ((n = utf8tods(encoding, 64, ud)) == (size_t)-1 ||
				(doc->u.Document.encoding = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		return;
	}
	doc->u.Document.standalone = standalone;
}
static void
doctype_start_fn(void *userData, const XML_Char *doctypeName, const XML_Char *sysid,
					const XML_Char *pubid, int has_internal_subset)
{
	struct user_data *ud = userData;
	DOM_Document *doc;
	DOM_DocumentType *doctype;

	has_internal_subset = 0;

	if (DOM_Exception) {
		return;
	}

	if ((doc = stack_peek(ud->stk)) == NULL || doc->u.Document.doctype) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (utf8tods(doctypeName, -1, ud) == (size_t)-1) {
		AMSG("");
		return;
	}
	if ((doctype = DOM_Implementation_createDocumentType(ud->buf, NULL, NULL)) == NULL) {
		AMSG("");
		return;
	}
	if (sysid && (utf8tods(sysid, -1, ud) == (size_t)-1 ||
				(doctype->u.DocumentType.systemId = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doc, doctype);
		return;
	}
	if (pubid && (utf8tods(pubid, -1, ud) == (size_t)-1 ||
				(doctype->u.DocumentType.publicId = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doc, doctype);
		return;
	}
	if (DOM_Node_appendChild(doc, doctype) == NULL) {
		AMSG("");
		return;
	}
	doc->u.Document.doctype = doctype;
	if (stack_push(ud->stk, doctype) == -1) {
		DOM_Exception = errno;
		AMSG("");
		return;
	}
}
static void
doctype_end_fn(void *userData)
{
	stack_pop(((struct user_data *)userData)->stk);
}
/*
static void
eldecl_content_write(void *userData, XML_Content *model, FILE *out)
{
	unsigned int i;
	switch (model->type) {
		case XML_CTYPE_EMPTY:
			fputs("EMPTY", out);
			break;
		case XML_CTYPE_ANY:
			fputs("ANY", out);
			break;
		case XML_CTYPE_MIXED:
			fputs("(#PCDATA", out);
			for (i = 0; i < model->numchildren; i++) {
				fputs("|", out);
				eldecl_content_write(userData, model->children + i, out);
			}
			fputc(')', stdout);
			if (model->quant == XML_CQUANT_REP) {
				fputc('*', out);
			}
			break;
		case XML_CTYPE_NAME:
			fputs(model->name, out);
			if (model->quant != XML_CQUANT_NONE) {
				fputc(quants[model->quant], out);
			}
			break;
		case XML_CTYPE_CHOICE:
		case XML_CTYPE_SEQ:
			fputc('(', stdout);
			if (model->numchildren) {
				eldecl_content_write(userData, model->children, out);
				for (i = 1; i < model->numchildren; i++) {
					fputs(model->type == XML_CTYPE_SEQ ? ", " : "|", out);
					eldecl_content_write(userData, model->children + i, out);
				}
			}
			fputc(')', stdout);
			if (model->quant != XML_CQUANT_NONE) {
				fputc(quants[model->quant], out);
			}
			break;
	}
}
*/
static void
eldecl_fn(void *userData, const XML_Char *name, XML_Content *model)
{
	userData = NULL; name = NULL;
/*
	printf("   <!ELEMENT %s ", name); 
	eldecl_content_write(userData, model, stdout);
	fputs(">\n", stdout);
*/
	free(model);
}
static void
attdecl_fn(void *userData, const XML_Char *elname, const XML_Char *attname,
			const XML_Char *att_type, const XML_Char *dflt, int isrequired)
{
	userData = NULL; elname = NULL; attname = NULL; att_type = NULL; dflt = NULL, isrequired = 0;
/*
	printf("   <!ATTLIST %s %s %s", elname, attname, att_type);
	if (isrequired) {
		if (dflt) {
			printf(" #FIXED \"%s\"", dflt);
		} else {
			printf(" #REQUIRED");
		}
	} else if (dflt) {
		printf(" \"%s\"", dflt);
	} else {
		printf(" #IMPLIED");
	}
	printf(">\n");
*/
}
static void
entdecl_fn(void *userData, const XML_Char *entityName, int is_parameter_entity,
		const XML_Char *value, int value_length, const XML_Char *base,
		const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName)
{
	struct user_data *ud = userData;
	DOM_DocumentType *doctype;
	DOM_Entity *entity;

	base = NULL;

	if (DOM_Exception || is_parameter_entity) {
		return;
	}

	if ((doctype = stack_peek(ud->stk)) == NULL) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if ((entity = Document_createNode(doctype->ownerDocument, DOM_ENTITY_NODE)) == NULL) {
		AMSG("");
		return;
	}

	if ((utf8tods(entityName, -1, ud) == (size_t)-1 ||
				(entity->nodeName = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, entity);
		return;
	}
	/* This is wrong. The nodeValue of an entity is Null. The children
	 * represent the value of an Entity. Not sure how this is supposed to be
	 * implemented with Expat. To be continued ...
	 */
	/* Possible expat bug. Value is NULL but value_length is 1 with the input:
	 * <!ENTITY ent5 PUBLIC "entityURI" "entityFile" NDATA notation1>
	 */
	if (value && (utf8tods(value, value_length, ud) == (size_t)-1 ||
				(entity->nodeValue = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		return;
	}
	if (publicId && (utf8tods(publicId, -1, ud) == (size_t)-1 ||
				(entity->u.Entity.publicId = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, entity);
		return;
	}
	if (systemId && (utf8tods(systemId, -1, ud) == (size_t)-1 ||
				(entity->u.Entity.systemId = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, entity);
		return;
	}
	if (notationName && (utf8tods(notationName, -1, ud) == (size_t)-1 ||
				(entity->u.Entity.notationName = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, entity);
		return;
	}
	if (DOM_Node_appendChild(doctype, entity) == NULL) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, entity);
		return;
	}
}
static void
notationdecl_fn(void *userData, const XML_Char *notationName, const XML_Char *base,
		const XML_Char *systemId, const XML_Char *publicId)
{
	struct user_data *ud = userData;
	DOM_DocumentType *doctype;
	DOM_Notation *notation;

	base = NULL;

	if (DOM_Exception) {
		return;
	}

	if ((doctype = stack_peek(ud->stk)) == NULL) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if ((notation = Document_createNode(doctype->ownerDocument, DOM_NOTATION_NODE)) == NULL) {
		AMSG("");
		return;
	}

	if ((utf8tods(notationName, -1, ud) == (size_t)-1 ||
				(notation->nodeName = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, notation);
		return;
	}
	if (publicId && (utf8tods(publicId, -1, ud) == (size_t)-1 ||
				(notation->u.Notation.publicId = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, notation);
		return;
	}
	if (systemId && (utf8tods(systemId, -1, ud) == (size_t)-1 ||
				(notation->u.Notation.systemId = mbsdup(ud->buf)) == NULL)) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, notation);
		return;
	}
	if (DOM_Node_appendChild(doctype, notation) == NULL) {
		AMSG("");
		DOM_Document_destroyNode(doctype->ownerDocument, notation);
		return;
	}
}
static void
start_fn(void *userData, const XML_Char *name, const XML_Char **atts)
{
	struct user_data *ud = userData;
	DOM_Node *parent, *child;
	DOM_String *a;
	int i;

	if (DOM_Exception) {
		return;
	}

	if (ud == NULL || name == NULL || atts == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		return;
	}

	parent = stack_peek(ud->stk);
	if (parent == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		return;
	}
	if (utf8tods(name, -1, ud) == (size_t)-1) {
		AMSG("name=%s", name);
		return;
	}
	if ((child = DOM_Document_createElement(parent->ownerDocument, ud->buf)) == NULL) {
		AMSG("");
		return;
	}
	if (DOM_Node_appendChild(parent, child) == NULL) {
		AMSG("");
		return;
	}
	for (i = 0; atts[i]; i += 2) {
		if (utf8tods(atts[i], -1, ud) == (size_t)-1 || (a = mbsdup(ud->buf)) == NULL) {
			AMSG("");
			return;
		}
		if (utf8tods(atts[i + 1], -1, ud) == (size_t)-1) {
			AMSG("");
			return;
		}
		DOM_Element_setAttribute(child, a, ud->buf);
		free(a);
		if (DOM_Exception) {
			return;
		}
	}
	if (stack_push(ud->stk, child) == -1) {
		DOM_Exception = errno;
		AMSG("");
		return;
	}
}
static void
end_fn(void *userData, const XML_Char *name)
{
	stack_pop(((struct user_data *)userData)->stk);
	name = NULL;
}
static void
cdata_start_fn(void *userData)
{
	((struct user_data *)userData)->cdata = 1;
}
static void
cdata_end_fn(void *userData)
{
	((struct user_data *)userData)->cdata = 0;
}
static void
chardata_fn(void *userData, const XML_Char *s, int len)
{
	struct user_data *ud = userData;
	DOM_Text *tex;
	DOM_Node *parent;

	if (DOM_Exception) {
		return;
	}

	if (ud == NULL || s == NULL || len == 0) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}

	parent = stack_peek(ud->stk);
	if (parent == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNO(DOM_Exception);
		return;
	}

	if (utf8tods(s, len, ud) == (size_t)-1) {
		AMSG("");
		return;
	}
	if (ud->cdata) {
		if ((tex = DOM_Document_createCDATASection(parent->ownerDocument, ud->buf)) == NULL) {
			AMSG("");
			return;
		}
	} else {
		if ((tex = DOM_Document_createTextNode(parent->ownerDocument, ud->buf)) == NULL) {
			AMSG("");
			return;
		}
	}

	DOM_Node_appendChild(parent, tex);
	if (DOM_Exception) {
		DOM_Document_destroyNode(parent->ownerDocument, tex);
	}
}
static void
comment_fn(void *userData, const XML_Char *s)
{
	struct user_data *ud = userData;
	DOM_Comment *com;
	DOM_Node *parent;

	if (DOM_Exception) {
		return;
	}

	parent = stack_peek(ud->stk);
	if (parent == NULL) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (utf8tods(s, -1, ud) == (size_t)-1) {
		AMSG("");
		return;
	}
	if ((com = DOM_Document_createComment(parent->ownerDocument, ud->buf))) {
		DOM_Node_appendChild(parent, com);
		if (DOM_Exception) {
			DOM_Document_destroyNode(parent->ownerDocument, com);
		}
	}
}
static void
processing_fn(void *userData, const XML_Char *target, const XML_Char *data)
{
	struct user_data *ud = userData;
	DOM_ProcessingInstruction *pi;
	DOM_Node *parent;
	DOM_String *targ;

	if (DOM_Exception) {
		return;
	}

	parent = stack_peek(ud->stk);
	if (parent == NULL) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return;
	}
	if (utf8tods(target, -1, ud) == (size_t)-1 || (targ = mbsdup(ud->buf)) == NULL) {
		AMSG("");
		return;
	}
	if (utf8tods(data, -1, ud) == (size_t)-1) {
		AMSG("");
		return;
	}
	if ((pi = DOM_Document_createProcessingInstruction(parent->ownerDocument, targ, ud->buf))) {
		DOM_Node_appendChild(parent, pi);
		if (DOM_Exception) {
			DOM_Document_destroyNode(parent->ownerDocument, pi);
		}
	}
	free(targ);
}
void
default_fn(void *userData, const XML_Char *s, int len)
{
	userData = NULL; s = NULL; len = 0;
}
int
DOM_DocumentLS_fread(DOM_DocumentLS *doc, FILE *stream)
{
	XML_Parser p;
	struct user_data ud;
	void *buf;
	size_t n;
	int ret, done;

	if (doc == NULL || stream == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNF(DOM_Exception, ": doc=%p,stream=%p", doc, stream);
		return -1;
	}

	p = XML_ParserCreate(NULL);
	if (p == NULL) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNO(DOM_Exception);
		return -1;
	}

	ud.buf = NULL;
	ud.siz = 0;
	ud.stk = stack_new(500, NULL);
	ud.cdata = 0;

	doc->ownerDocument = doc;

	if (ud.stk == NULL || stack_push(ud.stk, doc) == -1) {
		DOM_Exception = DOM_XML_PARSER_ERR;
		PMNF(DOM_Exception, ": stk=%p", ud.stk);
		XML_ParserFree(p);
		stack_del(ud.stk, NULL, NULL);
		return -1;
	}

	/*XML_SetDefaultHandler(p, default_fn);*/
	XML_SetElementHandler(p, start_fn, end_fn);
	XML_SetCharacterDataHandler(p, chardata_fn);
	XML_SetCdataSectionHandler(p, cdata_start_fn, cdata_end_fn);
	XML_SetCommentHandler(p, comment_fn);
	XML_SetProcessingInstructionHandler(p , processing_fn);
	XML_SetXmlDeclHandler(p, xmldecl_fn);
	XML_SetDoctypeDeclHandler(p, doctype_start_fn, doctype_end_fn);
	XML_SetElementDeclHandler(p, eldecl_fn);
	XML_SetAttlistDeclHandler(p, attdecl_fn);
	XML_SetEntityDeclHandler(p, entdecl_fn);
	XML_SetNotationDeclHandler(p, notationdecl_fn);
	XML_SetUserData(p, &ud);

	ret = -1;
	for ( ;; ) {
		if ((buf = XML_GetBuffer(p, BUFSIZ)) == NULL) {
			DOM_Exception = DOM_XML_PARSER_ERR;
			PMNF(DOM_Exception, ": buf=NULL");
			break;
		}
		if ((n = fread(buf, 1, BUFSIZ, stream)) == 0 && ferror(stream)) {
			DOM_Exception = ferror(stream);
			PMNO(DOM_Exception);
			break;
		}
		if (XML_ParseBuffer(p, n, (done = feof(stream))) == 0 || DOM_Exception) {
			if (DOM_Exception == 0) {
				DOM_Exception = DOM_XML_PARSER_ERR;
				PMNF(DOM_Exception, ": %s: line %u",
							XML_ErrorString(XML_GetErrorCode(p)),
							XML_GetCurrentLineNumber(p));
			} else {
				AMSG("");
			}
			break;
		}
		if (done) {
			ret = 0;
			break;
		}
	}

	DOM_Element_normalize(doc->u.Document.documentElement);

	free(ud.buf);
	stack_del(ud.stk, NULL, NULL);
	XML_ParserFree(p);

	doc->ownerDocument = NULL;

	return ret;
}
int
DOM_DocumentLS_load(DOM_DocumentLS *doc, const char *uri)
{
	FILE *fd;
	int ret;

	if (doc == NULL || uri == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNF(DOM_Exception, ": doc=%p,uri=%s", doc, uri);
		return -1;
	}

	fd = fopen(uri, "r");
	if (fd == NULL) {
		DOM_Exception = errno;
		PMNF(DOM_Exception, ": uri=%s", uri);
		return -1;
	}

	ret = DOM_DocumentLS_fread(doc, fd);

	fclose(fd);

	return ret;
}

#endif /* HAVE_EXPAT */

static void
fputds_encoded(const DOM_String *s, FILE *stream)
{
    size_t l;
    
    while (*s) {
        l = strcspn(s, "<>&\"");
        if (l > 0) {
            fwrite((void*)s, 1, sizeof(DOM_String) * l, stream);
            s += l;
        }
        switch (*s) {
        case '\0':
            break;
        case '<':
            fputs("&lt;", stream);
            break;
        case '>':
            fputs("&gt;", stream);
            break;
        case '&':
            fputs("&apos;", stream);
            break;
        case '"':
            fputs("&quot;", stream);
            break;
        default:
            AMSG("");
            break;
        };
        if(*s)
            ++s;
    }
}

int
DOM_DocumentLS_fwrite(const DOM_DocumentLS *node, FILE *stream)
{
	NodeEntry *e;
	DOM_Node *c;

    if (node == NULL || stream == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNF(DOM_Exception, ": node=%p,stream=%p", node, stream);
		return -1;
    }

	if (DOM_Exception) {
		return -1;
	}

    switch (node->nodeType) {
        case DOM_ELEMENT_NODE:
			fputc('<', stream);
			fputds(node->nodeName, stream);
			for (e = node->attributes->first; e != NULL; e = e->next) {
				fputc(' ', stream);
				fputds(e->node->nodeName, stream);
				fputs("=\"", stream);
				fputds_encoded(e->node->nodeValue, stream);
				fputc('"', stream);
			}
			if (DOM_Node_hasChildNodes(node)) {
				fputc('>', stream);
				for (c = node->firstChild; c != NULL; c = c->nextSibling) {
					if (DOM_DocumentLS_fwrite(c, stream) == -1) {
						/* Don't put msgno macro here or might overrun buf */
						return -1;
					}
            	}
				fputs("</", stream);
				fputds(node->nodeName, stream);
				fputc('>', stream);
			} else {
				fputs("/>", stream);
			}
            break;
        case DOM_ATTRIBUTE_NODE:
            break;
        case DOM_TEXT_NODE:
			fputds_encoded(node->nodeValue, stream);
            break;
        case DOM_CDATA_SECTION_NODE:
            break;
        case DOM_ENTITY_REFERENCE_NODE:
            break;
        case DOM_NOTATION_NODE:
			fputs("    <!NOTATION ", stream);
			fputds(node->nodeName, stream);
			if (node->u.Entity.publicId) {
				fputs(" PUBLIC \"", stream);
				fputds(node->u.Entity.publicId, stream);
				fputs("\" \"", stream);
				fputds(node->u.Entity.systemId, stream);
				fputc('"', stream);
			} else if (node->u.Entity.systemId) {
				fputs(" SYSTEM \"", stream);
				fputds(node->u.Entity.systemId, stream);
				fputc('"', stream);
			}
			fputs(">", stream);
			break;
        case DOM_ENTITY_NODE:
			fputs("    <!ENTITY ", stream);
			fputds(node->nodeName, stream);
			if (node->nodeValue) {
				fputc('"', stream);
				fputds(node->nodeValue, stream);
				fputc('"', stream);
			} else {
				if (node->u.Entity.publicId) {
					fputs(" PUBLIC \"", stream);
					fputds(node->u.Entity.publicId, stream);
					fputs("\" \"", stream);
					fputds(node->u.Entity.systemId, stream);
					fputc('"', stream);
				} else if (node->u.Entity.systemId) {
					fputs(" SYSTEM \"", stream);
					fputds(node->u.Entity.systemId, stream);
					fputc('"', stream);
				}
				if (node->u.Entity.notationName) {
					fputs(" NDATA ", stream);
					fputds(node->u.Entity.notationName, stream);
				}
			}
			fputs(">", stream);
            break;
        case DOM_PROCESSING_INSTRUCTION_NODE:
			fputs("<?", stream);
			fputds(node->u.ProcessingInstruction.target, stream);
			fputc(' ', stream);
			fputds_encoded(node->u.ProcessingInstruction.data, stream);
			fputs("?>", stream);
            break;
        case DOM_COMMENT_NODE:
			fputs("<!--", stream);
			fputds_encoded(node->nodeValue, stream);
			fputs("-->", stream);
            break;
        case DOM_DOCUMENT_NODE:
			fputs("<?xml", stream);
    		fputs(" version=\"", stream);
			fputds(node->u.Document.version ? node->u.Document.version : "1.0", stream);
			fputc('\"', stream);
#ifdef CODESET
			fputs(" encoding=\"", stream);
			fputs(nl_langinfo(CODESET), stream);
			fputc('\"', stream);
#endif
			if (node->u.Document.standalone != 0) {
				fputs(" standalone=\"yes\"", stream);
			}
			fputs("?>"NL, stream);
			for (c = node->firstChild; c != NULL; c = c->nextSibling) {
				if (DOM_DocumentLS_fwrite(c, stream) == -1) {
					AMSG("");
					return -1;
				}
           	}
			fputs(NL, stream);
            break;
        case DOM_DOCUMENT_TYPE_NODE:
			fputs(NL"<!DOCTYPE ", stream);
			fputs(node->u.DocumentType.name, stream);
			if (node->u.DocumentType.systemId) {
				fputs(" SYSTEM \"", stream);
				fputds(node->u.DocumentType.systemId, stream);
				fputc('"', stream);
			} else if (node->u.DocumentType.publicId) {
				fputs(" PUBLIC \"", stream);
				fputds(node->u.DocumentType.publicId, stream);
				fputc('"', stream);
			}
			if (node->u.DocumentType.internalSubset) {
				fputs(" ["NL, stream);
				fputds(node->u.DocumentType.internalSubset, stream);
				fputs("]>"NL, stream);
			} else {
				fputs(">"NL, stream);
			}
            break;
        case DOM_DOCUMENT_FRAGMENT_NODE:
            break;
    }

	return DOM_Exception ? -1 : 0;
}
int
DOM_DocumentLS_save(DOM_DocumentLS *doc, const char *uri, const DOM_Node *node)
{
	FILE *fd;

	if ((doc == NULL && node == NULL) || uri == NULL) {
		DOM_Exception = NULL_POINTER_ERR;
		PMNF(DOM_Exception, ": doc=%p,uri=%s,node=%p", doc, uri, node);
		return -1;
	}

	fd = fopen(uri, "w");
	if (fd && DOM_DocumentLS_fwrite(doc ? doc : node, fd) == 0) {
		fclose(fd);
		return 0;
	}
	DOM_Exception = errno;
	PMNF(DOM_Exception, ": uri=%s", uri);

	return -1;
}

