/* Test interger and time functions
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <encdec.h>

#if defined(_WIN32)
#define m1800888999000LL 1800888999000Ui64
#define mn999000999000LL -999000999000Ui64
#define m0x70F0F0F0F0F0F0F0LL 0x70F0F0F0F0F0F0F0Ui64
#define mn9LL -9Ui64
#else
#define m1800888999000LL 1800888999000LL
#define mn999000999000LL -999000999000LL
#define m0x70F0F0F0F0F0F0F0LL 0x70F0F0F0F0F0F0F0LL
#define mn9LL -9LL
#endif

unsigned char buf[1024];

struct data_block {
	short sbe0;
	short sbe1;
	short sbe2;
	short sle0;
	short sle1;
	short sle2;
	int ibe0;
	int ibe1;
	int ibe2;
	int ile0;
	int ile1;
	int ile2;
	uint64_t lbe0;
	uint64_t lbe1;
	uint64_t lbe2;
	uint64_t lle0;
	uint64_t lle1;
	uint64_t lle2;
	float fbe0;
	float fbe1;
	float fbe2;
	float fle0;
	float fle1;
	float fle2;
	double dbe0;
	double dbe1;
	double dbe2;
	double dle0;
	double dle1;
	double dle2;
	time_t tbe0;
	time_t tle0;
	time_t tbe1;
	time_t tle1;
	time_t tle2;
	time_t tbe2;
	time_t tle3;
	time_t tbe3;
};

int
main(int argc, char *argv[])
{
	unsigned char *p;
	struct data_block d0, d1;

	p = buf;
	d0.sbe0 = 1;
	d0.sbe1 = 2345;
	d0.sbe2 = -2;
	d0.sle0 = 7;
//printf("d0=%d\n", d0.sle0);
	d0.sle1 = 3456;
	d0.sle2 = -5;
	d0.ibe0 = 1;
	d0.ibe1 = 700000;
	d0.ibe2 = -233457;
	d0.ile0 = 7;
	d0.ile1 = 700500;
	d0.ile2 = -5;
	d0.lbe0 = 1;
	d0.lbe1 = m1800888999000LL;
	d0.lbe2 = mn999000999000LL;
	d0.lle0 = 1;
	d0.lle1 = m0x70F0F0F0F0F0F0F0LL;
	d0.lle2 = mn9LL;

	d0.fbe0 = -1.92727e-07;
	d0.fbe1 = 3.388282e+38;
	d0.fbe2 = 1.172828e-38;
	d0.fle0 = -1.92727e-07;
	d0.fle1 = 3.388282e+38;
	d0.fle2 = 1.172828e-38;
	d0.dbe0 = -2.218356e-16;
	d0.dbe1 = 1.7782822e+308;
	d0.dbe2 = 2.2100000e-308;
	d0.dle0 = -2.218356e-16;
	d0.dle1 = 1.7782822e+308;
	d0.dle2 = 2.2100000e-308;

	d0.tbe0 = d0.tle0 = d0.tbe1 = d0.tle1 = d0.tle2 = d0.tbe2 = d0.tbe3 = d0.tle3 = time(NULL);

	/*   0 0x00 */
	p += enc_uint16be(d0.sbe0, p);
	p += enc_uint16be(d0.sbe1, p);
	p += enc_uint16be(d0.sbe2, p);
	p += enc_uint16le(d0.sle0, p);
	p += enc_uint16le(d0.sle1, p);
	p += enc_uint16le(d0.sle2, p);
	/*  12 0x0C */
	p += enc_uint32be(d0.ibe0, p);
	p += enc_uint32be(d0.ibe1, p);
	p += enc_uint32be(d0.ibe2, p);
	p += enc_uint32le(d0.ile0, p);
	p += enc_uint32le(d0.ile1, p);
	p += enc_uint32le(d0.ile2, p);
	/*  36 0x24 */
	p += enc_uint64be(d0.lbe0, p);
	p += enc_uint64be(d0.lbe1, p);
	p += enc_uint64be(d0.lbe2, p);
	p += enc_uint64le(d0.lle0, p);
	p += enc_uint64le(d0.lle1, p);
	p += enc_uint64le(d0.lle2, p);
	/*  84 0x54 */
	p += enc_floatbe(d0.fbe0, p);
	p += enc_floatbe(d0.fbe1, p);
	p += enc_floatbe(d0.fbe2, p);
	p += enc_floatle(d0.fle0, p);
	p += enc_floatle(d0.fle1, p);
	p += enc_floatle(d0.fle2, p);
	/* 108 0x6C */
	p += enc_doublebe(d0.dbe0, p);
	p += enc_doublebe(d0.dbe1, p);
	p += enc_doublebe(d0.dbe2, p);
	p += enc_doublele(d0.dle0, p);
	p += enc_doublele(d0.dle1, p);
	p += enc_doublele(d0.dle2, p);
	/* 156 0x9C */
	p += enc_time(&d0.tbe0, p, TIME_1970_SEC_32BE);
	p += enc_time(&d0.tle0, p, TIME_1970_SEC_32LE);
	p += enc_time(&d0.tbe1, p, TIME_1904_SEC_32BE);
	p += enc_time(&d0.tle1, p, TIME_1904_SEC_32LE);
	p += enc_time(&d0.tle2, p, TIME_1601_NANOS_64LE);
	p += enc_time(&d0.tbe2, p, TIME_1601_NANOS_64BE);
	p += enc_time(&d0.tle3, p, TIME_1970_MILLIS_64LE);
	p += enc_time(&d0.tbe3, p, TIME_1970_MILLIS_64BE);
    /* 188 0xBC */

	p = buf;

	d1.sbe0 = dec_uint16be(p); p += 2;
	d1.sbe1 = dec_uint16be(p); p += 2;
	d1.sbe2 = dec_uint16be(p); p += 2;
	d1.sle0 = dec_uint16le(p); p += 2;
//printf("d1=%d\n", d1.sle0);
	d1.sle1 = dec_uint16le(p); p += 2;
	d1.sle2 = dec_uint16le(p); p += 2;

	d1.ibe0 = dec_uint32be(p); p += 4;
	d1.ibe1 = dec_uint32be(p); p += 4;
	d1.ibe2 = dec_uint32be(p); p += 4;
	d1.ile0 = dec_uint32le(p); p += 4;
	d1.ile1 = dec_uint32le(p); p += 4;
	d1.ile2 = dec_uint32le(p); p += 4;

	d1.lbe0 = dec_uint64be(p); p += 8;
	d1.lbe1 = dec_uint64be(p); p += 8;
	d1.lbe2 = dec_uint64be(p); p += 8;
	d1.lle0 = dec_uint64le(p); p += 8;
	d1.lle1 = dec_uint64le(p); p += 8;
	d1.lle2 = dec_uint64le(p); p += 8;

	d1.fbe0 = dec_floatbe(p); p += 4;
	d1.fbe1 = dec_floatbe(p); p += 4;
	d1.fbe2 = dec_floatbe(p); p += 4;
	d1.fle0 = dec_floatle(p); p += 4;
	d1.fle1 = dec_floatle(p); p += 4;
	d1.fle2 = dec_floatle(p); p += 4;

	d1.dbe0 = dec_doublebe(p); p += 8;
	d1.dbe1 = dec_doublebe(p); p += 8;
	d1.dbe2 = dec_doublebe(p); p += 8;
	d1.dle0 = dec_doublele(p); p += 8;
	d1.dle1 = dec_doublele(p); p += 8;
	d1.dle2 = dec_doublele(p); p += 8;

	d1.tbe0 = dec_time(p, TIME_1970_SEC_32BE); p += 4;
	d1.tle0 = dec_time(p, TIME_1970_SEC_32LE); p += 4;
	d1.tbe1 = dec_time(p, TIME_1904_SEC_32BE); p += 4;
	d1.tle1 = dec_time(p, TIME_1904_SEC_32LE); p += 4;
	d1.tle2 = dec_time(p, TIME_1601_NANOS_64LE); p += 8;
	d1.tbe2 = dec_time(p, TIME_1601_NANOS_64BE); p += 8;
	d1.tle3 = dec_time(p, TIME_1970_MILLIS_64LE); p += 8;
	d1.tbe3 = dec_time(p, TIME_1970_MILLIS_64BE); p += 8;

	assert(d0.sbe0 == d1.sbe0);
	assert(d0.sbe1 == d1.sbe1);
	assert(d0.sbe2 == d1.sbe2);
	assert(d0.sle0 == d1.sle0);
	assert(d0.sle1 == d1.sle1);
	assert(d0.sle2 == d1.sle2);

	assert(d0.ibe0 == d1.ibe0);
	assert(d0.ibe1 == d1.ibe1);
	assert(d0.ibe2 == d1.ibe2);
	assert(d0.ile0 == d1.ile0);
	assert(d0.ile1 == d1.ile1);
	assert(d0.ile2 == d1.ile2);

	assert(d0.lbe0 == d1.lbe0);
	assert(d0.lbe1 == d1.lbe1);
	assert(d0.lbe2 == d1.lbe2);
	assert(d0.lle0 == d1.lle0);
	assert(d0.lle1 == d1.lle1);
	assert(d0.lle2 == d1.lle2);

	assert(d0.fbe0 == d1.fbe0);
	assert(d0.fbe1 == d1.fbe1);
	assert(d0.fbe2 == d1.fbe2);
	assert(d0.fle0 == d1.fle0);
	assert(d0.fle1 == d1.fle1);
	assert(d0.fle2 == d1.fle2);

	assert(d0.dbe0 == d1.dbe0);
	assert(d0.dbe1 == d1.dbe1);
	assert(d0.dbe2 == d1.dbe2);
	assert(d0.dle0 == d1.dle0);
	assert(d0.dle1 == d1.dle1);
	assert(d0.dle2 == d1.dle2);

	assert(d0.tbe0 == d1.tbe0);
	assert(d0.tle0 == d1.tle0);
	assert(d0.tbe1 == d1.tbe1);
	assert(d0.tle1 == d1.tle1);
	assert(d0.tle2 == d1.tle2);
	assert(d0.tbe2 == d1.tbe2);
	assert(d0.tle3 == d1.tle3);
	assert(d0.tbe3 == d1.tbe3);

	fputs(ctime(&d0.tbe0), stderr);
	fputs(ctime(&d1.tbe0), stderr);

	if (argc > 1) {
		FILE *f = fopen(argv[1],"wb");
		if(f == NULL || fwrite(buf, 1, p - buf, f) != (p - buf)) {
			fprintf(stderr, "Failed to write file: %s\n", argv[1]);
			return EXIT_FAILURE;
		}
	}

	fprintf(stderr, "Success\n");

	return EXIT_SUCCESS;
}

