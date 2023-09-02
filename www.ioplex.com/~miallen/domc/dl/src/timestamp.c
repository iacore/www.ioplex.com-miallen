#if defined(_WIN32)
#include <Windows.h>

typedef unsigned __int64 uint64_t;

uint64_t
timestamp(void)
{
	FILETIME ftime;
	uint64_t ret;

	GetSystemTimeAsFileTime(&ftime);

	ret = ftime.dwHighDateTime;
	ret <<= 32Ui64;
	ret |= ftime.dwLowDateTime;

	return ret;
}

#else
#include <sys/time.h>
#include "defines.h"
#include <mba/msgno.h>
#include "domc.h"

uint64_t
timestamp(void)
{
	struct timeval tval;

	if (gettimeofday(&tval, NULL) < 0) {
		DOM_Exception = errno;
		PMNO(DOM_Exception);
		return 1;
	}
	return tval.tv_sec * 1000LL + tval.tv_usec / 1000;
}

#endif

