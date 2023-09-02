#ifndef DEFINES_H
#define DEFINES_H

#if defined(__sparc__)

#define NL "\n"
#define HAVE_ENCDEC 0
#define HAVE_STRDUP 1
#define HAVE_STRNLEN 0
#define HAVE_EXPAT 195
#define HAVE_MBSTATE 0
#define HAVE_WCWIDTH 1
#define HAVE_SNPRINTF 1
#define HAVE_VARMACRO 1

#elif defined(_WIN32)

#define NL "\r\n"
#define HAVE_ENCDEC 0
#define HAVE_STRDUP 1
#define HAVE_STRNLEN 1
#define HAVE_EXPAT 195
#define HAVE_MBSTATE 0
#define HAVE_WCWIDTH 0
#define HAVE_SNPRINTF 0
#define HAVE_VARMACRO 0

#else

#define NL "\n"
#define HAVE_ENCDEC 0
#define HAVE_STRDUP 1
#define HAVE_STRNLEN 1
#define HAVE_EXPAT 195
#define HAVE_MBSTATE 1
#define HAVE_WCWIDTH 1
#define HAVE_SNPRINTF 1
#define HAVE_VARMACRO 1

#endif
#endif /* DEFINES_H */
