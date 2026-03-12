#ifndef USLEEP_H
#define USLEEP_H

#include <windows.h>

static inline void usleep(unsigned int usec) 
{ 
    HANDLE timer = NULL; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*(__int64)usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    if (timer == NULL) {
        // Fallback: CreateWaitableTimer failed, use Sleep as best-effort
        Sleep((usec + 999) / 1000);
        return;
    }

    if (!SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0)) {
        // Setting timer failed -> cleanup and fallback
        CloseHandle(timer);
        Sleep((usec + 999) / 1000);
        return;
    }

    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}

#endif