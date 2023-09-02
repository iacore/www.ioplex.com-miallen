#ifndef MBS_H
#define MBS_H

/* mbs - locale dependent multi-byte string functions
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOMC_API
#ifdef WIN32
# ifdef DOMC_EXPORTS
#  define DOMC_API  __declspec(dllexport)
# else /* DOMC_EXPORTS */
#  define DOMC_API  __declspec(dllimport)
# endif /* DOMC_EXPORTS */
#else /* WIN32 */
# define DOMC_API extern
#endif /* WIN32 */
#endif /* DOMC_API */

#include <stddef.h>
#include <wchar.h>

DOMC_API int mbslen(const char *src);
DOMC_API int mbsnlen(const char *src, size_t sn, int cn);

DOMC_API size_t mbssize(const char *src);
DOMC_API size_t mbsnsize(const char *src, size_t sn, int cn);

DOMC_API char *mbsdup(const char *src);
DOMC_API char *mbsndup(const char *src, size_t n, int cn);

DOMC_API char *mbsoff(char *src, int off);
DOMC_API char *mbsnoff(char *src, int off, size_t sn);

DOMC_API char *mbschr(char *src, wchar_t wc);
DOMC_API char *mbsnchr(char *src, size_t sn, int cn, wchar_t wc);

DOMC_API int mbswidth(const char *src, size_t sn, int wn);

DOMC_API char *mbssub(char *src, size_t sn, int wn);

#ifdef __cplusplus
}
#endif

#endif /* MBS_H */
