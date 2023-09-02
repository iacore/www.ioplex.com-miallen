#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <expat.h>

int expatdump_err = 0;

#define EXPATDUMP_XML_PARSER_ERR 1

static void
start_fn(void *userData, const XML_Char *name, const XML_Char **atts)
{
	printf("<%s>", name);
}
static void
end_fn(void *userData, const XML_Char *name)
{
	printf("</%s>", name);
}
static void
doctype_start_fn(void *userData, const XML_Char *doctypeName, const XML_Char *sysid,
					const XML_Char *pubid, int has_internal_subset)
{
	printf("DOCTYPE %s [\n", doctypeName);
}
static void
doctype_end_fn(void *userData)
{
	printf("]\n");
}
static void
eldecl_fn(void *userData, const XML_Char *name, XML_Content *model)
{
	printf("ELEMENT %s\n", name); 
	free(model);
}
static void
entdecl_fn(void *userData, const XML_Char *entityName, int is_parameter_entity,
		const XML_Char *value, int value_length, const XML_Char *base,
		const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName)
{
	printf("ENTITY %s\n", entityName);
}

static int parse(XML_Parser p, FILE *stream);

static int
extern_fn(XML_Parser p, const XML_Char *context, const XML_Char *base,
			const XML_Char *systemId, const XML_Char *publicId)
{
	XML_Parser ep;
	FILE *in;
	int ret;

	if ((in = fopen(systemId, "r")) == NULL) {
		expatdump_err = errno;
		return 0;
	}

	if ((ep = XML_ExternalEntityParserCreate(p, context, NULL)) == NULL) {
		expatdump_err = EXPATDUMP_XML_PARSER_ERR;
		fclose(in);
		return 0;
	}

	ret = 1;
	if (parse(ep, in) == -1) {
		ret = 0;
	}

	fclose(in);

	return ret;
}
static int
parse(XML_Parser p, FILE *stream)
{
	int ret, pret, done;
	void *buf;
	size_t n;

	ret = 0;
	for ( ;; ) {
		if ((buf = XML_GetBuffer(p, BUFSIZ)) == NULL) {
			expatdump_err = EXPATDUMP_XML_PARSER_ERR;
			ret = -1;
			break;
		}
		if ((n = fread(buf, 1, BUFSIZ, stream)) == 0 && ferror(stream)) {
			expatdump_err = ferror(stream);
			ret = -1;
			break;
		}
		pret = XML_ParseBuffer(p, n, (done = feof(stream)));
		if (pret == 0 || expatdump_err) {
			int err = XML_GetErrorCode(p);
			if (pret == 0 && err != XML_ERROR_EXTERNAL_ENTITY_HANDLING) {
fprintf(stderr, "Error parsing external entity: %s\n", XML_ErrorString(err));
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
	int ret;

	if ((p = XML_ParserCreate(NULL)) == NULL) {
		expatdump_err = EXPATDUMP_XML_PARSER_ERR;
		return -1;
	}

	XML_SetElementHandler(p, start_fn, end_fn);
	XML_SetDoctypeDeclHandler(p, doctype_start_fn, doctype_end_fn);
	XML_SetElementDeclHandler(p, eldecl_fn);
	XML_SetEntityDeclHandler(p, entdecl_fn);
	XML_SetExternalEntityRefHandler(p, extern_fn);
	XML_SetParamEntityParsing(p, XML_PARAM_ENTITY_PARSING_ALWAYS);

	ret = 0;
	if (parse(p, stream) == -1) {
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
		fprintf(stderr, "Failure\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

