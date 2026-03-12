#include "tdbtimes.h"

#ifdef _WIN32
#include <sys/timeb.h>
#include <time.h>
#include <windows.h>
#else
#error "This implementation is for Windows only."
#endif

int gettimeofday(struct timeval* tv, void* tz)
{
    (void)tz; // suppress unused parameter warning

    if (tv == NULL) {
        return -1;
    }

    FILETIME ft;
    ULARGE_INTEGER uli;

    // Windows file time: 100-ns intervals since 1601-01-01 UTC
    GetSystemTimeAsFileTime(&ft);

    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // Convert to Unix epoch: 1970-01-01 UTC
    const unsigned long long EPOCH_DIFF = 116444736000000000ULL;
    unsigned long long time64 = uli.QuadPart - EPOCH_DIFF;

    tv->tv_sec = (long)(time64 / 10000000ULL);
    tv->tv_usec = (long)((time64 % 10000000ULL) / 10);

    return 0;
}

clock_t times(struct tms* buf)
{
    if (buf == NULL) {
        return (clock_t)-1;
    }

    buf->tms_utime = clock();
    buf->tms_stime = 0;
    buf->tms_cutime = 0;
    buf->tms_cstime = 0;

    return buf->tms_utime;
}