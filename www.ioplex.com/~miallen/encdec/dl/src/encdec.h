#ifndef ENCDEC_H
#define ENCDEC_H

/* encdec - encode and decode integers, times, and
 * internationalized strings to and from popular binary formats
 */

#include <stddef.h>
#include <time.h>

#if defined(__sparc__)
  #include <sys/inttypes.h>
#elif defined(_WIN32)
  typedef unsigned short uint16_t;
  typedef unsigned long uint32_t;
  typedef unsigned __int64 uint64_t;
#else
  #include <stdint.h>
#endif


/* Decode bit fields
 */

#define FLD(i, m) (m != 0 ? ((i) & (m)) / ((m) & -(m)) : 0)

/* Encode integers
 */

size_t enc_uint16be(uint16_t s, unsigned char *dst);
size_t enc_uint32be(uint32_t i, unsigned char *dst);
size_t enc_uint16le(uint16_t s, unsigned char *dst);
size_t enc_uint32le(uint32_t i, unsigned char *dst);

/* Decode integers
 */

uint16_t dec_uint16be(const unsigned char *src);
uint32_t dec_uint32be(const unsigned char *src);
uint16_t dec_uint16le(const unsigned char *src);
uint32_t dec_uint32le(const unsigned char *src);

/* Encode and decode 64 bit integers
 */

size_t enc_uint64be(uint64_t i, unsigned char *dst);
size_t enc_uint64le(uint64_t i, unsigned char *dst);
uint64_t dec_uint64be(const unsigned char *src);
uint64_t dec_uint64le(const unsigned char *src);

/* Encode floating point numbers
 */

size_t enc_floatle(const float f, unsigned char *dst);
size_t enc_doublele(const double d, unsigned char *dst);
size_t enc_floatbe(const float f, unsigned char *dst);
size_t enc_doublebe(const double d, unsigned char *dst);

/* Decode floating point numbers
 */

float dec_floatle(const unsigned char *src);
double dec_doublele(const unsigned char *src);
float dec_floatbe(const unsigned char *src);
double dec_doublebe(const unsigned char *src);

/* Encode times
 */

enum encdec_time_encodings {
	TIME_1970_SEC_32BE = 1,
	TIME_1970_SEC_32LE,
	TIME_1904_SEC_32BE,
	TIME_1904_SEC_32LE,
	TIME_1601_NANOS_64LE,
	TIME_1601_NANOS_64BE,
	TIME_1970_MILLIS_64BE,
	TIME_1970_MILLIS_64LE,
	TIME_NUM_ENC
};

size_t enc_time(const time_t *timep, unsigned char *dst, int enc);

/* Decode times
 */

time_t dec_time(const unsigned char *src, int enc);

/* Encode strings
 */

/* #define EINSEQ_SUBS 0xFFFD */
#define EINSEQ_SUBS 0x003F

int enc_mbscpy(const char *src, char **dst, const char *tocode);
int enc_mbsncpy(const char *src, size_t sn, char **dst, size_t dn,
                                    int cn, const char *tocode);

/* Decode strings
 */

char *dec_mbsdup(char **src, const char *fromcode);
char *dec_mbsndup(char **src, size_t sn, size_t dn,
                                    int wn, const char *fromcode);
size_t dec_mbscpy(char **src, char *dst, const char *fromcode);
size_t dec_mbsncpy(char **src, size_t sn, char *dst, size_t dn,
                                    int cn, const char *fromcode);


#endif /* ENCDEC_H */

