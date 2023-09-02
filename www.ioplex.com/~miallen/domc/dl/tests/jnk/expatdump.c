#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mba/msgno.h>
#include <expat.h>

#define CTX_WIDTH 80

static char quants[] = { 0, '?', '*', '+' };

int expatdump_err = 0;

struct msgno_entry expatdump_codes[] = {
	{ 1, "An XML parser error occured" },
	{ 0, NULL }
};

#define EXPATDUMP_XML_PARSER_ERR expatdump_codes[0].msgno

typedef struct {
	int depth;
} ParserData;

static void
start_fn(void *userData, const XML_Char *name, const XML_Char **atts)
{
	int i;

	printf("<%s", name);
	for (i = 0; atts[i]; i += 2) {
		printf(" %s=\"%s\"", atts[i], atts[i + 1]);
	}
	printf(">");
}
static void
end_fn(void *userData, const XML_Char *name)
{
	printf("</%s>", name);
}
static void
chardata_fn(void *userData, const XML_Char *s, int len)
{
	fwrite(s, 1, len, stdout);
}
static void
cdata_start_fn(void *userData)
{
	printf("<![CDATA[");
}
static void
cdata_end_fn(void *userData)
{
	printf("]]>");
}
static void
comment_fn(void *userData, const XML_Char *s)
{
	printf("<!--%s-->", s);
}
static void
processing_fn(void *userData, const XML_Char *target, const XML_Char *data)
{
	printf("<?%s %s?>", target, data);
}
static void
xmldecl_fn(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone)
{
	if (version == NULL) {
		printf("TEXT DECLARATION");
	}
	printf("<?xml version=\"%s\"", version);
	if (encoding) {
		printf(" encoding=\"%s\"", encoding);
	}
	if (standalone == 0) {
		printf(" standalone=\"no\"");
	} else if (standalone == 1) {
		printf(" standalone=\"no\"");
	}
	printf("?>");
}
static void
doctype_start_fn(void *userData, const XML_Char *doctypeName, const XML_Char *sysid,
					const XML_Char *pubid, int has_internal_subset)
{
	printf("\n<?DOCTYPE %s", doctypeName);
	if (sysid) {
		printf(" SYSTEM \"%s\"", sysid);
	} else if (pubid) {
		printf(" PUBLIC \"%s\"", pubid);
	}
	if (has_internal_subset) {
		fputs(" [\n", stdout);
	}
}
static void
doctype_end_fn(void *userData)
{
	printf("]>");
}
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
static void
eldecl_fn(void *userData, const XML_Char *name, XML_Content *model)
{
	printf("   <!ELEMENT %s ", name); 
	eldecl_content_write(userData, model, stdout);
	fputs(">\n", stdout);
	free(model);
}
static void
attdecl_fn(void *userData, const XML_Char *elname, const XML_Char *attname,
			const XML_Char *att_type, const XML_Char *dflt, int isrequired)
{
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
}
static void
entdecl_fn(void *userData, const XML_Char *entityName, int is_parameter_entity,
		const XML_Char *value, int value_length, const XML_Char *base,
		const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName)
{
	printf("   <!ENTITY %s", entityName);
	if (value) {
		fputs(" \"", stdout);
		fwrite(value, 1, value_length, stdout);
		fputs("\"", stdout);
	} else {
		if (publicId) {
			printf(" PUBLIC \"%s\" \"%s\"", publicId, systemId);
		} else if (systemId) {
			printf(" SYSTEM \"%s\"", systemId);
		}
		if (notationName) {
			printf(" NDATA %s", notationName);
		}
	}
	fputs(">\n", stdout);
}
static void
notationdecl_fn(void *userData, const XML_Char *notationName, const XML_Char *base,
		const XML_Char *systemId, const XML_Char *publicId)
{
	printf("   <!NOTATION %s", notationName);
	if (base) {
		printf(" %s", base);
	}
	if (publicId) {
		if (systemId) {
			printf(" PUBLIC \"%s\" \"%s\"", publicId, systemId);
		} else {
			printf(" PUBLIC \"%s\"", publicId);
		}
	} else if (systemId) {
		printf(" SYSTEM \"%s\"", systemId);
	}
	fputs(">\n", stdout);
}
static int
parse(XML_Parser p, FILE *stream)
{
	void *buf;
	size_t n;
	int ret, pret, done;

	ret = 0;
	for ( ;; ) {
		if ((buf = XML_GetBuffer(p, BUFSIZ)) == NULL) {
			expatdump_err = EXPATDUMP_XML_PARSER_ERR;
			PMNO(expatdump_err);
			ret = -1;
			break;
		}
		if ((n = fread(buf, 1, BUFSIZ, stream)) == 0 && ferror(stream)) {
			expatdump_err = ferror(stream);
			PMNO(expatdump_err);
			ret = -1;
			break;
		}
		pret = XML_ParseBuffer(p, n, (done = feof(stream)));
		if (pret == 0 || pret == XML_ERROR_EXTERNAL_ENTITY_HANDLING || expatdump_err) {
			int err = XML_GetErrorCode(p);
			if (pret == 0 && err != XML_ERROR_EXTERNAL_ENTITY_HANDLING) {
				int off, beg, siz, i;
				const char *ctx;
				char buf[2 * CTX_WIDTH];

				expatdump_err = EXPATDUMP_XML_PARSER_ERR;
				ctx = XML_GetInputContext(p, &off, &siz);
				beg = 0;
				if (off > (CTX_WIDTH / 2)) {
					beg = off - CTX_WIDTH / 2;
				}
				siz -= beg;
				if (siz > CTX_WIDTH) {
					siz = CTX_WIDTH;
				}
				strncpy(buf, ctx + beg, siz);
				for (i = 0; i < siz; i++) {
					if (buf[i] == '\n') {
						buf[i] = ' ';
					}
				}
				buf[siz - 1] = '\n';
				off -= beg;
				for (i = 0; i < off; i++) {
					buf[siz + i] = ' ';
				}
				buf[siz + i++] = '^';
				buf[siz + i] = '\0';

				PMNF(expatdump_err, ": line %d col %d off %d: %s:\n%s", XML_GetCurrentLineNumber(p),
							XML_GetCurrentColumnNumber(p), off, XML_ErrorString(err), buf);
			} else {
				AMSG("");
			}
			ret = -1;
			break;
		}
		if (done) {
			break;
		}
	}

	return ret;
}
int
expatdump(FILE *stream)
{
	XML_Parser p;
	ParserData pd;
	void *buf;
	size_t n;
	int ret, done;

	msgno_add_codes(expatdump_codes);

	if ((p = XML_ParserCreate(NULL)) == NULL) {
		expatdump_err = EXPATDUMP_XML_PARSER_ERR;
		PMNO(expatdump_err);
		return -1;
	}

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

	memset(&pd, 0, sizeof(ParserData));
	XML_SetUserData(p, &pd);

	ret = 0;
	if (parse(p, stream) == -1) {
		AMSG("");
		ret = -1;
	}

	XML_ParserFree(p);

	return ret;
}

int
main(int argc, const char *argv[])
{
	FILE *in;

	errno = 0;
	if (argc == 2) {
		if ((in = fopen(argv[1], "r")) == NULL) {
			perror("fopen");
			return EXIT_FAILURE;
		}
	} else if (argc == 1) {
		in = stdin;
	} else {
		fprintf(stderr, "usage: expatdump <xmlfilename>\n");
		return EXIT_FAILURE;
	}

	if (expatdump(in) == -1) {
		MSG("Failure");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

