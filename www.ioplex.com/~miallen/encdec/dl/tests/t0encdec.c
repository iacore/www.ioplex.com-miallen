/* Test UTF-8 string operations
 *
 * ./t0encdec.sh 2>&1 | grep Fail
 */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <encdec.h>

char fbuf[0xFFFF];
char b0[8192];
char b1[8192];

struct record {
	char *str;
	size_t len;
	int nchars;
};

void
wipe(void)
{
	int n = 0;
	while (n < 8192) {
		b0[n++] = 0xee;
	} 
	n = 0;
	while (n < 8192) {
		b1[n++] = 0xee;
	} 
}

void
hexdump(FILE *stream, const void *src, size_t len, size_t width)
{
	unsigned int rows, pos, c, i;
	const char *start, *rowpos, *data = (char *)src;

	start = data;
	pos = 0;
	rows = (len % width) == 0 ? len / width : len / width + 1;
	for (i = 0; i < rows; i++) {
		rowpos = data;
		fprintf(stream, "%05x: ", pos);
		do {
			c = *data++ & 0xff;
			if ((data - start) <= len) {
				fprintf(stream, " %02x", c);
			} else {
				fprintf(stream, "   ");
			}
		} while(((data - rowpos) % width) != 0);
		fprintf(stream, "  |");
		data -= width;
		do {
			c = *data++;
			if (isprint(c) == 0) {
				c = '.';
			}
			if ((data - start) <= len) {
				fprintf(stream, "%c", c);
			} else {
				fprintf(stream, " ");
			}
		} while(((data - rowpos) % width) != 0);
		fprintf(stream, "|\n");
		pos += width;
	}
}

int
run(struct record *rec, const char *enc)
{
	char *src;
	char *dst;
	size_t rsn, rdn, rcn, sn, dn, cn;
	int i;
	size_t n, j;

	printf("UTF-8 <-> %s: len=%u,nchars=%u,str=\n", enc, rec->len, rec->nchars);
	hexdump(stdout, rec->str, rec->len, 16);

	rsn = rec->len;
	rcn = rec->nchars;

/* sn cn */

	src = rec->str;
	sn = rsn;
	dst = b0;
	dn = 8192;
	cn = rcn;

	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if (i != cn) {
		printf("Failure: dst-b0=%u,i=%d,cn=%d\n", dst-b0, i, cn);
		if (i == -1) {
#ifndef __STDC_ISO_10646__
			printf("__STDC_ISO_10646__ is required for the encdec multi-byte string functions\n");
#endif
		}
		return 0;
	}

	src = b0;
	sn = rdn = dst - b0;
	dst = b1;
	dn = 8192;
	cn = rcn;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, rec->len) || n != rsn) {
		printf("Failure: n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
		hexdump(stdout, b1, n + 1, 16);
		return 0;
	} else {
		printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	}

	src = b1;
	dst = b0;

	i = enc_mbsncpy(src, -1, &dst, -1, -1, enc) - 1;
	if (i != rcn) {
		printf("Failure: i=%d,rcn=%d\n", i, rcn);
		hexdump(stdout, b0, dst - b0, 16);
		return 0;
	}

	src = b0;
	dst = b1;

	j = dec_mbsncpy(&src, -1, NULL, -1, -1, enc);
	if ((n = dec_mbsncpy(&src, -1, dst, -1, -1, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j);
hexdump(stdout, b1, n, 16);
		return 0;
	}

	if (strncmp(rec->str, b1, strlen(b1))) {
		printf("Failure:\n");
		hexdump(stdout, b1, n + 1, 16);
		return 0;
	}


/* dn */

	src = rec->str;
	sn = -1;
	dst = b0;
	dn = rdn;
	cn = -1;

	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if ((dst - b0) != dn) {
		printf("Failure: dst-b0=%u,dn=%u\n", dst-b0, dn);
		hexdump(stdout, b0, dst-b0+1, 16);
		return 0;
	}

	src = b0;
	sn = 8192;
	dst = b1;
	dn = rsn + 1;
	cn = -1;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, rec->len) || n != rsn) {
		printf("Failure: n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
		hexdump(stdout, b1, n + 1, 16);
		return 0;
	} else {
	/*	printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	 */
	}

/* sn dn cn */

	src = rec->str;
	sn = rsn;
	dst = b0;
	dn = rdn;
	cn = rcn;

	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if ((dst - b0) != dn) {
		printf("Failure: dst-b0=%u,i=%d,cn=%d\n", dst-b0, i, cn); return 0;
	}

	src = b0;
	sn = dst - b0;
	dst = b1;
	dn = rsn + 1;
	cn = rcn;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, rec->len) || n != rsn) {
		printf("Failure\n"); return 0;
	} else {
		printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	}
	hexdump(stdout, b1, n + 1, 16);

/* cn */

	src = rec->str;
	sn = -1;
	dst = b0;
	dn = 8192;
	cn = rcn;

	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if (i != cn) {
		printf("Failure\n"); return 0;
	}

	src = b0;
	sn = -1;
	dst = b1;
	dn = -1;
	cn = rcn;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, rec->len) || n != rsn) {
		printf("Failure: n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
		hexdump(stdout, b1, n + 1, 16);
		return 0;
	} else {
		printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	}
	hexdump(stdout, b1, n + 1, 16);

/* cn - N */

	src = rec->str;
	sn = -1;
	dst = b0;
	dn = 8192;
	cn = rcn - 1;

	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if (i != cn) {
		printf("Failure\n"); return 0;
	}

	src = b0;
	sn = -1;
	dst = b1;
	dn = 8192;
	cn = rcn - 1;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, n)) {
		printf("Failure\n"); return 0;
	} else {
		printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	}
	hexdump(stdout, b1, n + 1, 16);

/* sn - N */

	src = rec->str;
	sn = rsn - 1;
	dst = b0;
	dn = 8192;
	cn = -1;


	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if ((rcn - i) > 2) { /* This allows up to 2 combining
                                     * characters can be truncated.
                                     */
		printf("Failure: dst-b0=%u,i=%d,rcn=%d\n", dst-b0, i, rcn);
		hexdump(stdout, b0, dst-b0 + 1, 16);
		return 0;
	}

	src = b0;
	sn = dst - b0 - 1;
	dst = b1;
	dn = 8192;
	cn = -1;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, n) || (sn - (src - b0) > 3)) {
		printf("Failure: n=%u,rsn=%u,b1[n]=0x%02x,src-b0=%u,sn=%u\n",
                         n, rsn, b1[n] & 0xFF, src-b0, sn);
		hexdump(stdout, b1, n + 1, 16);
		return 0;
	} else {
		printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	}
	hexdump(stdout, b1, n + 1, 16);

/* dn - N */

	src = rec->str;
	sn = -1;
	dst = b0;
	dn = rdn - 1;
	cn = -1;

	i = enc_mbsncpy(src, sn, &dst, dn, cn, enc);
	if ((rcn - i) > 2) {
		printf("Failure: dst-b0=%u,rdn=%u,i=%d,rcn=%d,cn=%d\n", dst-b0, rdn, i, rcn, cn); return 0;
	}
printf("sn=%u,dn=%u,cn=%d -> dst-b0=%u,i=%d\n", sn, dn, cn, dst-b0, i);
hexdump(stdout, b0, dst-b0 + 1, 16);

	src = b0;
	sn = -1;
	dst = b1;
	dn = rsn - 2;
	cn = rcn;

	j = dec_mbsncpy(&src, sn, NULL, dn, cn, enc) - 1;
	if ((n = dec_mbsncpy(&src, sn, dst, dn, cn, enc)) != j) {
		printf("Failure: n=%u,j=%u\n", n, j); return 0;
	}
	if (strncmp(rec->str, b1, n) || (rsn - n) > 4) {
		printf("Failure: n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
		hexdump(stdout, b1, n + 1, 16);
		return 0;
	} else {
		printf("n=%u,rsn=%u,b1[n]=0x%02x\n", n, rsn, b1[n] & 0xFF);
	}
	hexdump(stdout, b1, n + 1, 16);

	printf("Success\n");
	return 1;
}

size_t
record_read(struct record *rec, char *src)
{
	const char *start;

	start = src;

	rec->nchars = strtol(src, NULL, 10);

	while (*src++ != '[') {
		;
	}

	rec->str = src;

	while (src[0] != ']' || src[1] != '\n') {
		src++;
	}

	rec->len = src - rec->str;

	return src - start + 2;
}
int
main(int argc, char *argv[])
{
	FILE *fd;
	size_t flen, n;
	struct record rec;
	int rn;

	if (argc < 3) {
		fprintf(stderr, "./t0encdec.o t0encdec.utf8 5\n");
		return EXIT_FAILURE;
	}

	if (!setlocale(LC_CTYPE, "en_US.UTF-8")) {
			fprintf(stderr, "Can't set the specified locale! "
							"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}

	rn = atoi(argv[2]);

	fd = fopen(argv[1], "rb");
	flen = fread(fbuf, 1, 0xFFFF, fd);

	errno = 0;

	n = 0;
	while (n < flen) {
		n += record_read(&rec, fbuf + n);
		if (rn-- == 0) run(&rec, "UCS-2BE");
	}

	return EXIT_SUCCESS;
}
