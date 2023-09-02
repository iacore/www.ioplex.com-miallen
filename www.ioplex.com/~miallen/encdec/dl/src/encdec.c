/* encdec - encode and decode integers, times, and
 * internationalized strings to and from popular binary formats
 * Copyright (c) 2002 Michael B. Allen <mballen@erols.com>
 *
 * The GNU Library General Public License
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#include <stdlib.h>
#include <errno.h>
#include "encdec.h"

#if defined(_WIN32)
#define m0xFFFFFFFFLL 0xFFFFFFFFUi64
#define m32LL 32Ui64
#define m1000LL 1000Ui64
#define m10000LL 10000Ui64
#define MILLISECONDS_BETWEEN_1970_AND_1601 11644473600000Ui64
#else
#define m0xFFFFFFFFLL 0xFFFFFFFFLL
#define m32LL 32LL
#define m1000LL 1000LL
#define m10000LL 10000LL
#define MILLISECONDS_BETWEEN_1970_AND_1601 11644473600000LL
#endif

union convert_float {
	float f;
	uint32_t i;
};
union convert_double {
	double d;
	uint64_t i;
};

/* Encode integers
 */

size_t
enc_uint16be(uint16_t s, unsigned char *dst)
{
	dst[0] = (s >> 8) & 0xFF;
	dst[1] = s & 0xFF;
	return 2;
}
size_t
enc_uint32be(uint32_t i, unsigned char *dst)
{
	dst[0] = (i >> 24) & 0xFF;
	dst[1] = (i >> 16) & 0xFF;
	dst[2] = (i >> 8) & 0xFF;
	dst[3] = i & 0xFF;
	return 4;
}
size_t
enc_uint16le(uint16_t s, unsigned char *dst)
{
	dst[0] = s & 0xFF;
	dst[1] = (s >> 8) & 0xFF;
	return 2;
}
size_t
enc_uint32le(uint32_t i, unsigned char *dst)
{
	dst[0] = i & 0xFF;
	dst[1] = (i >> 8) & 0xFF;
	dst[2] = (i >> 16) & 0xFF;
	dst[3] = (i >> 24) & 0xFF;
	return 4;
}

/* Decode integers
 */

uint16_t
dec_uint16be(const unsigned char *src)
{
	return ((unsigned)src[0] << 8) | src[1];
}
uint32_t
dec_uint32be(const unsigned char *src)
{
	return ((unsigned)src[0] << 24) | ((unsigned)src[1] << 16) |
           ((unsigned)src[2] << 8) | src[3];
}
uint16_t
dec_uint16le(const unsigned char *src)
{
	return src[0] | ((unsigned)src[1] << 8);
}
uint32_t
dec_uint32le(const unsigned char *src)
{
	return src[0] | ((unsigned)src[1] << 8) |
           ((unsigned)src[2] << 16) | ((unsigned)src[3] << 24);
}

/* Encode and decode 64 bit integers
 */

size_t
enc_uint64be(uint64_t i, unsigned char *dst)
{
	enc_uint32be(i & m0xFFFFFFFFLL, dst + 4);
	enc_uint32be((i >> m32LL) & m0xFFFFFFFFLL, dst);
	return 8;
}
size_t
enc_uint64le(uint64_t i, unsigned char *dst)
{
	enc_uint32le(i & m0xFFFFFFFFLL, dst);
	enc_uint32le((i >> m32LL) & m0xFFFFFFFFLL, dst + 4);
	return 8;
}
uint64_t
dec_uint64be(const unsigned char *src)
{
	uint64_t i;
	i = dec_uint32be(src);
	i <<= m32LL;
	i |= dec_uint32be(src + 4);
	return i;
}
uint64_t
dec_uint64le(const unsigned char *src)
{
	uint64_t i;
	i = dec_uint32le(src + 4);
	i <<= m32LL;
	i |= dec_uint32le(src);
	return i;
}

/* Encode floats
 */

size_t
enc_floatle(const float f, unsigned char *dst)
{
	union convert_float c;
	c.f = f;
	return enc_uint32le(c.i, dst);
}
size_t
enc_floatbe(const float f, unsigned char *dst)
{
	union convert_float c;
	c.f = f;
	return enc_uint32be(c.i, dst);
}

/* Decode floating point numbers
 */

float
dec_floatle(const unsigned char *src)
{
	union convert_float c;
	c.i = dec_uint32le(src);
	return c.f;
}
float
dec_floatbe(const unsigned char *src)
{
	union convert_float c;
	c.i = dec_uint32be(src);
	return c.f;
}


/* Encode and decode doubles
 */

size_t
enc_doublele(const double d, unsigned char *dst)
{
	union convert_double c;
	c.d = d;
	return enc_uint64le(c.i, dst);
}
size_t
enc_doublebe(const double d, unsigned char *dst)
{
	union convert_double c;
	c.d = d;
	return enc_uint64be(c.i, dst);
}
double
dec_doublele(const unsigned char *src)
{
	union convert_double c;
	c.i = dec_uint64le(src);
	return c.d;
}
double
dec_doublebe(const unsigned char *src)
{
	union convert_double c;
	c.i = dec_uint64be(src);
	return c.d;
}

/* Encode times
 */

#define SEC_BETWEEEN_1904_AND_1970 2082844800

size_t
enc_time(const time_t *timep, unsigned char *dst, int enc)
{
	uint64_t t;

	if (timep == NULL || dst == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (enc < 1 || enc > TIME_NUM_ENC) {
		errno = EINVAL;
		return -1;
	}

	switch(enc) {
		case TIME_1970_SEC_32BE:
			return enc_uint32be(*timep, dst);
		case TIME_1970_SEC_32LE:
			return enc_uint32le(*timep, dst);
		case TIME_1904_SEC_32BE:
			return enc_uint32be(*timep + SEC_BETWEEEN_1904_AND_1970, dst);
		case TIME_1904_SEC_32LE:
			return enc_uint32le(*timep + SEC_BETWEEEN_1904_AND_1970, dst);
		case TIME_1601_NANOS_64BE:
			t = ((*timep * m1000LL) + MILLISECONDS_BETWEEN_1970_AND_1601) * m10000LL;
			return enc_uint64be(t, dst);
		case TIME_1601_NANOS_64LE:
			t = ((*timep * m1000LL) + MILLISECONDS_BETWEEN_1970_AND_1601) * m10000LL;
			return enc_uint64le(t, dst);
		case TIME_1970_MILLIS_64BE:
			return enc_uint64be(*timep * m1000LL, dst);
		case TIME_1970_MILLIS_64LE:
			return enc_uint64le(*timep * m1000LL, dst);
		default:
			return (size_t)-1;
	}
}

/* Decode times
 */

time_t
dec_time(const unsigned char *src, int enc)
{
	uint64_t t;

	if (src == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (enc < 1 || enc > TIME_NUM_ENC) {
		errno = EINVAL;
		return -1;
	}

	switch(enc) {
		case TIME_1970_SEC_32BE:
			return dec_uint32be(src);
		case TIME_1970_SEC_32LE:
			return dec_uint32le(src);
		case TIME_1904_SEC_32BE:
			return dec_uint32be(src) - SEC_BETWEEEN_1904_AND_1970;
		case TIME_1904_SEC_32LE:
			return dec_uint32le(src) - SEC_BETWEEEN_1904_AND_1970;
		case TIME_1601_NANOS_64BE:
			t = dec_uint64be(src);
			return (t / m10000LL - MILLISECONDS_BETWEEN_1970_AND_1601) / m1000LL;
		case TIME_1601_NANOS_64LE:
			t = dec_uint64le(src);
			return (t / m10000LL - MILLISECONDS_BETWEEN_1970_AND_1601) / m1000LL;
		case TIME_1970_MILLIS_64BE:
			return dec_uint64be(src) / m1000LL;
		case TIME_1970_MILLIS_64LE:
			return dec_uint64le(src) / m1000LL;
		default:
			return (time_t)-1;
	}
}

