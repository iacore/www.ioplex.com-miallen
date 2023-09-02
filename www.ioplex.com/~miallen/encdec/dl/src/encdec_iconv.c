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

/* Modified iconv.c to support encdec multi-byte string
 * functions. This module can only be built directly
 * against libiconv-1.7/lib sources.
 */

#include <stdlib.h>
#include <iconv.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "config.h"
#include "libcharset.h"
#include "encdec.h"
#include "defines.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define BUFSIZ 512

#if HAVE_WCWIDTH > 0
int wcwidth(wchar_t ucs);
#else
#define wcwidth mk_wcwidth
int mk_wcwidth(wchar_t ucs);
#endif

/*
 * Consider those system dependent encodings that are needed for the
 * current system.
 */
#ifdef _AIX
#define USE_AIX
#endif
#ifdef __osf__
#define USE_OSF1
#endif
#ifdef __DJGPP__
#define USE_DOS
#endif

/*
 * Data type for general conversion loop.
 */
struct loop_funcs {
  size_t (*loop_convertn) (iconv_t icd,
                          const char* * inbuf, size_t *inbytesleft,
                          char* * outbuf, size_t *outbytesleft, int *n, int zt);
  size_t (*loop_reset) (iconv_t icd,
                        char* * outbuf, size_t *outbytesleft);
};

/*
 * Converters.
 */
#include "converters.h"

/*
 * Transliteration tables.
 */
#include "cjk_variants.h"
#include "translit.h"

/*
 * Table of all supported encodings.
 */
struct encoding {
  struct mbtowc_funcs ifuncs; /* conversion multibyte -> unicode */
  struct wctomb_funcs ofuncs; /* conversion unicode -> multibyte */
  int oflags;                 /* flags for unicode -> multibyte conversion */
};
enum {
#define DEFENCODING(xxx_names,xxx,xxx_ifuncs1,xxx_ifuncs2,xxx_ofuncs1,xxx_ofuncs2) \
  ei_##xxx ,
#include "encodings.def"
#ifdef USE_AIX
#include "encodings_aix.def"
#endif
#ifdef USE_OSF1
#include "encodings_osf1.def"
#endif
#ifdef USE_DOS
#include "encodings_dos.def"
#endif
#include "encodings_local.def"
#undef DEFENCODING
ei_for_broken_compilers_that_dont_like_trailing_commas
};
#include "flags.h"
static struct encoding const all_encodings[] = {
#define DEFENCODING(xxx_names,xxx,xxx_ifuncs1,xxx_ifuncs2,xxx_ofuncs1,xxx_ofuncs2) \
  { xxx_ifuncs1,xxx_ifuncs2, xxx_ofuncs1,xxx_ofuncs2, ei_##xxx##_oflags },
#include "encodings.def"
#ifdef USE_AIX
#include "encodings_aix.def"
#endif
#ifdef USE_OSF1
#include "encodings_osf1.def"
#endif
#ifdef USE_DOS
#include "encodings_dos.def"
#endif
#undef DEFENCODING
#define DEFENCODING(xxx_names,xxx,xxx_ifuncs1,xxx_ifuncs2,xxx_ofuncs1,xxx_ofuncs2) \
  { xxx_ifuncs1,xxx_ifuncs2, xxx_ofuncs1,xxx_ofuncs2, 0 },
#include "encodings_local.def"
#undef DEFENCODING
};

/*
 * Conversion loops.
 */
#include "loops.h"

/* Heavily modified libiconv-1.7/lib/loop_unicode.h
 */

static size_t
unicode_loop_convertn(iconv_t icd, const char **inbuf, size_t *inbytesleft,
                        char **outbuf, size_t *outbytesleft, int *n, int zt)
{
	conv_t cd = (conv_t)icd;
	size_t result = 0;
	const unsigned char *inptr = (const unsigned char *)*inbuf;
	size_t inleft = *inbytesleft;
	unsigned char *outptr = (unsigned char *)*outbuf;
	size_t outleft = *outbytesleft;

	static unsigned char buf[BUFSIZ];
	ucs4_t wc = 1;
	int incount, outcount;
	int count = *n, w;

	if (!count) {
		return 0;
	}

	while (wc && inleft && outleft > zt && count >= 0) {
		incount = cd->ifuncs.xxx_mbtowc(cd, &wc, inptr, inleft);
		if (incount < 0) {
			if (incount == RET_ILSEQ) {
				errno = EILSEQ;
				return -1;
			}
			if (incount == RET_TOOFEW(0)) {
				break;
			}
			incount = -2 - incount;
		} else {
			outcount = cd->ofuncs.xxx_wctomb(cd, buf, wc, MAX(outleft, BUFSIZ));
			if (outcount != RET_ILUNI) {
				goto outcount_ok;
			}
			if ((wc >> 7) == (0xe0000 >> 7)) {
				goto outcount_zero;
			}
			result++;
			if (cd->transliterate) {
				outcount = unicode_transliterate(cd, wc, buf, MAX(outleft, BUFSIZ));
				if (outcount != RET_ILUNI) {
					goto outcount_ok;
				}
			}
			outcount = cd->ofuncs.xxx_wctomb(cd, buf, EINSEQ_SUBS, MAX(outleft, BUFSIZ));
			if (outcount != RET_ILUNI) {
				goto outcount_ok;
			}
			errno = EILSEQ;
			return -1;
		outcount_ok:
			if (outcount < 0 ||
					outcount > outleft ||
					(zt && wc && outcount == outleft)) {
				break;
			}
			if (wc == 0 || (w = wcwidth(wc)) == -1) {
				w = 1;
			}
			if (w > count) { /* this allows count to be 0 while the loop
                              * continues so we pick up trailing combining chars
                              */
				break;
			}
			count -= w;
			if (*outbuf) {
				memmove(outptr, buf, outcount);
			}
			outptr += outcount;
			outleft -= outcount;
		outcount_zero: ;
		}
		if (!(incount <= inleft)) abort();
		inptr += incount;
		inleft -= incount;
	}
	if (*outbuf) {
		if (zt && wc) {
			cd->ofuncs.xxx_wctomb(cd, outptr, 0, 1);
		}
		*inbuf = (const char *)inptr;
	} else if (wc || count == 0) { /* if dst is NULL add a 1 to the return
                    * value but only if wc wasn't zero already in which case 1 will
                    * already have been added OR if count is 0 because we could have read
                    * a trailing '\0' acedentally which would make wc 0.
                    */
		outptr++;
	}
	*inbytesleft = inleft;
	*outbuf = outptr;
	*outbytesleft = outleft;
	*n = count;
	return result;
}

/* Copied from libiconv-1.7/lib/loop_wchar.h
 */

#if HAVE_WCRTOMB

/* From wchar_t to anything else. */

static size_t wchar_from_loop_convertn (iconv_t icd,
                                       const char* * inbuf, size_t *inbytesleft,
                                       char* * outbuf, size_t *outbytesleft, int *n, int zt)
{
  struct wchar_conv_struct * wcd = (struct wchar_conv_struct *) icd;
  size_t result = 0;
  while (*inbytesleft >= sizeof(wchar_t)) {
    const wchar_t * inptr = (const wchar_t *) *inbuf;
    size_t inleft = *inbytesleft;
    char buf[BUFSIZ];
#if HAVE_MBSTATE > 0
    mbstate_t state = wcd->state;
#endif
    size_t bufcount = 0;
    while (inleft >= sizeof(wchar_t)) {
      /* Convert one wchar_t to multibyte representation. */
#if HAVE_MBSTATE > 0
      size_t count = wcrtomb(buf+bufcount,*inptr,&state);
#else
      size_t count = wctomb(buf+bufcount,*inptr);
#endif
      if (count == (size_t)(-1)) {
        /* Invalid input. */
        errno = EILSEQ;
        return -1;
      }
      inptr++;
      inleft -= sizeof(wchar_t);
      bufcount += count;
      if (count == 0) {
        /* Continue, append next wchar_t. */
      } else {
        /* Attempt to convert the accumulated multibyte representations
           to the target encoding. */
        const char* bufptr = buf;
        size_t bufleft = bufcount;
        char* outptr = *outbuf;
        size_t outleft = *outbytesleft;
        int cnt = 1;
        size_t res = unicode_loop_convertn(&wcd->parent,
                                          &bufptr,&bufleft,
                                          &outptr,&outleft,&cnt,zt);
        if (res == (size_t)(-1)) {
          if (errno == EILSEQ)
            /* Invalid input. */
            return -1;
          else if (errno == E2BIG)
            /* Output buffer too small. */
            return -1;
          else if (errno == EINVAL) {
            /* Continue, append next wchar_t, but avoid buffer overrun. */
            if (bufcount + MB_CUR_MAX > BUFSIZ)
              abort();
          } else
            abort();
        } else {
          /* Successful conversion. */
#if HAVE_MBSTATE > 0
          wcd->state = state;
#endif
          *inbuf = (const char *) inptr;
          *inbytesleft = inleft;
          *outbuf = outptr;
          *outbytesleft = outleft;
          result += res;
          *n -= 1;
          break;
        }
      }
    }
  }
  return result;
}

#endif

#if HAVE_MBRTOWC

/* From anything else to wchar_t. */

static size_t wchar_to_loop_convertn (iconv_t icd,
                                     const char* * inbuf, size_t *inbytesleft,
                                     char* * outbuf, size_t *outbytesleft, int *n, int zt)
{
  struct wchar_conv_struct * wcd = (struct wchar_conv_struct *) icd;
  size_t result = 0;

  while (*inbytesleft > 0 && *n > 0) {
    size_t try;
    for (try = 1; try <= *inbytesleft; try++) {
      char buf[BUFSIZ];
      const char* inptr = *inbuf;
      size_t inleft = *inbytesleft;
      char* bufptr = buf;
      size_t bufleft = BUFSIZ;
      int cnt = 1;
      size_t res = unicode_loop_convertn(&wcd->parent,
                                        &inptr,&inleft,
                                        &bufptr,&bufleft,&cnt, zt);
      if (res == (size_t)(-1)) {
        if (errno == EILSEQ)
          /* Invalid input. */
          return -1;
        else if (errno == EINVAL) {
          /* Incomplete input. Next try with one more input byte. */
        } else
          /* E2BIG shouldn't occur. */
          abort();
      } else {
        /* Successful conversion. */
        size_t bufcount = bufptr-buf; /* = BUFSIZ-bufleft */
        wchar_t wc;
#if defined(HAVE_MBSTATE)
        mbstate_t state = wcd->state;
        res = mbrtowc(&wc,buf,bufcount,&state);
#else
        res = mbtowc(&wc,buf,bufcount);
#endif
        if (res == (size_t)(-2)) {
          /* Next try with one more input byte. */
        } else if (res == (size_t)(-1)) {
          /* Invalid input. */
          return -1;
        } else {
          if (*outbytesleft < sizeof(wchar_t)) {
            errno = E2BIG;
            return -1;
          }
          *(wchar_t*) *outbuf = wc;
          *outbuf += sizeof(wchar_t);
          *outbytesleft -= sizeof(wchar_t);
          *inbuf += try;
          *inbytesleft -= try;
          result += res;
          *n -= 1;
          break;
        }
      }
    }
  }
  return result;
}

#endif

/*
 * Alias lookup function.
 * Defines
 *   struct alias { const char* name; unsigned int encoding_index; };
 *   const struct alias * aliases_lookup (const char *str, unsigned int len);
 *   #define MAX_WORD_LENGTH ...
 */
#include "aliases.h"

/*
 * System dependent alias lookup function.
 * Defines
 *   const struct alias * aliases2_lookup (const char *str);
 */
#if defined(USE_AIX) || defined(USE_OSF1) || defined(USE_DOS) /* || ... */
static struct alias sysdep_aliases[] = {
#ifdef USE_AIX
#include "aliases_aix.h"
#endif
#ifdef USE_OSF1
#include "aliases_osf1.h"
#endif
#ifdef USE_DOS
#include "aliases_dos.h"
#endif
};
#ifdef __GNUC__
__inline
#endif
const struct alias *
aliases2_lookup (register const char *str)
{
  struct alias * ptr;
  unsigned int count;
  for (ptr = sysdep_aliases, count = sizeof(sysdep_aliases)/sizeof(sysdep_aliases[0]); count > 0; ptr++, count--)
    if (!strcmp(str,ptr->name))
      return ptr;
  return NULL;
}
#else
#define aliases2_lookup(str)  NULL
#endif

#if 0
/* Like !strcasecmp, except that the both strings can be assumed to be ASCII
   and the first string can be assumed to be in uppercase. */
static int strequal (const char* str1, const char* str2)
{
  unsigned char c1;
  unsigned char c2;
  for (;;) {
    c1 = * (unsigned char *) str1++;
    c2 = * (unsigned char *) str2++;
    if (c1 == 0)
      break;
    if (c2 >= 'a' && c2 <= 'z')
      c2 -= 'a'-'A';
    if (c1 != c2)
      break;
  }
  return (c1 == c2);
}
#endif

int
enc_mbscpy(const char *inbuf, char **outbuf, const char *tocode)
{
	return enc_mbsncpy(inbuf, INT_MAX, outbuf, INT_MAX, INT_MAX, tocode);
}
int
enc_mbsncpy(const char *inbuf, size_t inbytesleft, char **outbuf, size_t outbytesleft,
											int n, const char *tocode)
{
  struct conv_struct * cd;
  char buf[MAX_WORD_LENGTH+10+1];
  const char* cp, *fromcode;
  char* bp;
  const struct alias * ap;
  unsigned int count;
  unsigned int from_index;
  int from_wchar;
  unsigned int to_index;
  int to_wchar;
  int transliterate = 0;
  int start;

  if (inbytesleft > INT_MAX) {
    inbytesleft = INT_MAX;
  }
  if (outbytesleft > INT_MAX) {
    outbytesleft = INT_MAX;
  }
  if (n < 0) {
    n = INT_MAX;
  }
  start = n;

  /* Before calling aliases_lookup, convert the input string to upper case,
   * and check whether it's entirely ASCII (we call gperf with option "-7"
   * to achieve a smaller table) and non-empty. If it's not entirely ASCII,
   * or if it's too long, it is not a valid encoding name.
   */
  for (to_wchar = 0;;) {
    /* Search tocode in the table. */
    for (cp = tocode, bp = buf, count = MAX_WORD_LENGTH+10+1; ; cp++, bp++) {
      unsigned char c = * (unsigned char *) cp;
      if (c >= 0x80)
        goto invalid;
      if (c >= 'a' && c <= 'z')
        c -= 'a'-'A';
      *bp = c;
      if (c == '\0')
        break;
      if (--count == 0)
        goto invalid;
    }
    if (bp-buf > 10 && memcmp(bp-10,"//TRANSLIT",10)==0) {
      bp -= 10;
      *bp = '\0';
      transliterate = 1;
    }
    ap = aliases_lookup(buf,bp-buf);
    if (ap == NULL) {
      ap = aliases2_lookup(buf);
      if (ap == NULL)
        goto invalid;
    }
    if (ap->encoding_index == ei_local_char) {
      tocode = locale_charset();
      continue;
    }
    if (ap->encoding_index == ei_local_wchar_t) {
#if __STDC_ISO_10646__
      if (sizeof(wchar_t) == 4) {
        to_index = ei_ucs4internal;
        break;
      }
      if (sizeof(wchar_t) == 2) {
        to_index = ei_ucs2internal;
        break;
      }
      if (sizeof(wchar_t) == 1) {
        to_index = ei_iso8859_1;
        break;
      }
#endif
#if HAVE_MBRTOWC
      to_wchar = 1;
      tocode = locale_charset();
      continue;
#endif
      goto invalid;
    }
    to_index = ap->encoding_index;
    break;
  }

  fromcode = locale_charset();
  ap = aliases_lookup(fromcode, strlen(fromcode));
  if (ap == NULL) {
    ap = aliases2_lookup(fromcode);
    if (ap == NULL)
      goto invalid;
  }
  from_wchar = 0;
  from_index = ap->encoding_index;

  cd = (struct conv_struct *) malloc(from_wchar != to_wchar
                                     ? sizeof(struct wchar_conv_struct)
                                     : sizeof(struct conv_struct));
  if (cd == NULL) {
    errno = ENOMEM;
    return -1;
  }
  cd->iindex = from_index;
  cd->ifuncs = all_encodings[from_index].ifuncs;
  cd->oindex = to_index;
  cd->ofuncs = all_encodings[to_index].ofuncs;
  cd->oflags = all_encodings[to_index].oflags;
  /* Initialize the loop functions. */
#if HAVE_MBRTOWC
  if (to_wchar) {
    cd->lfuncs.loop_convertn = wchar_to_loop_convertn;
    cd->lfuncs.loop_reset = wchar_to_loop_reset;
  } else
#endif
  {
    cd->lfuncs.loop_convertn = unicode_loop_convertn;
    cd->lfuncs.loop_reset = unicode_loop_reset;
  }
  /* Initialize the states. */
  memset(&cd->istate,'\0',sizeof(state_t));
  memset(&cd->ostate,'\0',sizeof(state_t));
  /* Initialize the operation flags. */
  cd->transliterate = transliterate;
  /* Initialize additional fields. */
  if (from_wchar != to_wchar) {
    struct wchar_conv_struct * wcd = (struct wchar_conv_struct *) cd;
#if HAVE_MBSTATE > 0
    memset(&wcd->state,'\0',sizeof(mbstate_t));
#endif
  }

  if (inbuf == NULL)
    goto invalid;

  cd->lfuncs.loop_convertn(cd, (const char **)&inbuf, &inbytesleft, outbuf, &outbytesleft, &n, 0);

  free(cd);

  /* Done. */
  return start - n;
invalid:
  errno = EINVAL;
  return -1;
}

char *
dec_mbsdup(char **src, const char *fromcode)
{
	return dec_mbsndup(src, -1, -1, -1, fromcode);
}
char *
dec_mbsndup(char **src, size_t sn, size_t dn, int wn, const char *fromcode)
{
	size_t n;
	char *dst;

	if ((n = dec_mbsncpy(src, sn, NULL, dn, wn, fromcode)) == (size_t)-1) {
		return NULL;
	}
	if ((dst = malloc(n)) == NULL) {
		return NULL;
	}
	if (dec_mbsncpy(src, sn, dst, dn, wn, fromcode) == (size_t)-1) {
		free(dst);
		return NULL;
	}

	return dst;
}
size_t dec_mbscpy(char **inbuf, char *outbuf, const char *fromcode)
{
	return dec_mbsncpy(inbuf, INT_MAX, outbuf, INT_MAX, INT_MAX, fromcode);
}
size_t dec_mbsncpy(char **inbuf, size_t inbytesleft,
              char *outbuf, size_t outbytesleft, int n, const char *fromcode)
{
  struct conv_struct * cd;
  char buf[MAX_WORD_LENGTH+10+1];
  const char* cp, *tocode;
  char* bp, *start;
  const struct alias * ap;
  unsigned int count;
  unsigned int from_index;
  int from_wchar;
  unsigned int to_index;
  int to_wchar;
  int transliterate = 0;
  size_t r;

  if (inbytesleft > INT_MAX) {
    inbytesleft = INT_MAX;
  }
  if (outbytesleft > INT_MAX) {
    outbytesleft = INT_MAX;
  }
  if (n < 0) {
    n = INT_MAX;
  }

  /* Before calling aliases_lookup, convert the input string to upper case,
   * and check whether it's entirely ASCII (we call gperf with option "-7"
   * to achieve a smaller table) and non-empty. If it's not entirely ASCII,
   * or if it's too long, it is not a valid encoding name.
   */
  for (from_wchar = 0;;) {
    /* Search fromcode in the table. */
    for (cp = fromcode, bp = buf, count = MAX_WORD_LENGTH+10+1; ; cp++, bp++) {
      unsigned char c = * (unsigned char *) cp;
      if (c >= 0x80)
        goto invalid;
      if (c >= 'a' && c <= 'z')
        c -= 'a'-'A';
      *bp = c;
      if (c == '\0')
        break;
      if (--count == 0)
        goto invalid;
    }
    if (bp-buf > 10 && memcmp(bp-10,"//TRANSLIT",10)==0) {
      bp -= 10;
      *bp = '\0';
    }
    ap = aliases_lookup(buf,bp-buf);
    if (ap == NULL) {
      ap = aliases2_lookup(buf);
      if (ap == NULL)
        goto invalid;
    }
    if (ap->encoding_index == ei_local_char) {
      fromcode = locale_charset();
      continue;
    }
    if (ap->encoding_index == ei_local_wchar_t) {
#if __STDC_ISO_10646__
      if (sizeof(wchar_t) == 4) {
        from_index = ei_ucs4internal;
        break;
      }
      if (sizeof(wchar_t) == 2) {
        from_index = ei_ucs2internal;
        break;
      }
      if (sizeof(wchar_t) == 1) {
        from_index = ei_iso8859_1;
        break;
      }
#endif
#if HAVE_WCRTOMB
      from_wchar = 1;
      fromcode = locale_charset();
      continue;
#endif
      goto invalid;
    }
    from_index = ap->encoding_index;
    break;
  }

  tocode = locale_charset();
  ap = aliases_lookup(tocode, strlen(tocode));
  if (ap == NULL) {
    ap = aliases2_lookup(tocode);
    if (ap == NULL) {
      goto invalid;
    }
  }
  to_wchar = 0;
  to_index = ap->encoding_index;

  cd = (struct conv_struct *) malloc(from_wchar != to_wchar
                                     ? sizeof(struct wchar_conv_struct)
                                     : sizeof(struct conv_struct));
  if (cd == NULL) {
    errno = ENOMEM;
    return -1;
  }
  cd->iindex = from_index;
  cd->ifuncs = all_encodings[from_index].ifuncs;
  cd->oindex = to_index;
  cd->ofuncs = all_encodings[to_index].ofuncs;
  cd->oflags = all_encodings[to_index].oflags;
  /* Initialize the loop functions. */
#if HAVE_WCRTOMB
  if (from_wchar) {
    cd->lfuncs.loop_convertn = wchar_from_loop_convertn;
    cd->lfuncs.loop_reset = wchar_from_loop_reset;
  } else
#endif
  {
    cd->lfuncs.loop_convertn = unicode_loop_convertn;
    cd->lfuncs.loop_reset = unicode_loop_reset;
  }
  /* Initialize the states. */
  memset(&cd->istate,'\0',sizeof(state_t));
  memset(&cd->ostate,'\0',sizeof(state_t));
  /* Initialize the operation flags. */
  cd->transliterate = transliterate;
  /* Initialize additional fields. */
  if (from_wchar != to_wchar) {
    struct wchar_conv_struct * wcd = (struct wchar_conv_struct *) cd;
#if HAVE_MBSTATE > 0
    memset(&wcd->state,'\0',sizeof(mbstate_t));
#endif
  }

  start = outbuf;
  r = cd->lfuncs.loop_convertn(cd, (const char **)inbuf, &inbytesleft,
											&outbuf, &outbytesleft, &n, 1);

  free(cd);

  /* Done. */
  return r == (size_t)-1 ? -1 : outbuf - start;
invalid:
  errno = EINVAL;
  return -1;
}

