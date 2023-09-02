/* Test UTF-8 loop conversion
 *
 *  iconv -f UTF-8 -t UCS-2LE utf8.html > ucs2le.html
 *  LANG=en_US.UTF-8 ./t1encdec.o ucs2le.html UCS-2LE > utf8.out
 *  diff utf8.html utf8.out
 */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <encdec.h>

#define INSIZE 17
#define OUTSIZE 15

char inbuf[INSIZE];
char outbuf[OUTSIZE];

int
loop(FILE *in, FILE *out, const char *enc)
{
	size_t leftover = 0, sn, n;
	char *src;
	int done;

	do {
		sn = leftover + fread(inbuf + leftover, 1, INSIZE - leftover, in);
		if ((sn + 2) < INSIZE && feof(in)) {
			inbuf[sn++] = 0;                 /* must be '\0' terminated */
			inbuf[sn++] = 0;
		}
		src = inbuf;
		if ((n = dec_mbsncpy(&src, sn, outbuf, OUTSIZE, -1, enc)) == (size_t)-1) {
			return 0;
		}
		leftover = (inbuf + sn) - src;
		memmove(inbuf, src, leftover);
		done = outbuf[n - 1] == '\0';

		fwrite(outbuf, 1, n - done, out);
	} while (!done);

	return 1;
}

int
main(int argc, char *argv[])
{
	FILE *fd;

	if (argc < 3) {
		fprintf(stderr, "LANG=en_US.utf8 ./t1encdec.o utf8.html UTF-8 > out; diff utf8.html out\n");
		return EXIT_FAILURE;
	}

	if (!setlocale(LC_CTYPE, "")) {
			fprintf(stderr, "Can't set the specified locale! "
							"Check LANG, LC_CTYPE, LC_ALL.\n");
		return EXIT_FAILURE;
	}

	fd = fopen(argv[1], "rb");
	if (loop(fd, stdout, argv[2]) == 0) {
		fprintf(stderr, "loop conversion failed\n");
		return EXIT_FAILURE;
	}
	fflush(stdout);
	fclose(fd);

	fprintf(stderr, "Success\n");

	return EXIT_SUCCESS;
}

