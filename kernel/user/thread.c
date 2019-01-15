#include "thread.h"

#include <kernel/user/process_manager.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

Thread * GetCurrentThread()
{
    return gCurrentThread;
}

Thread * GetThreadFromPid(int tid)
{
    if (tid < 0)
    {
        KLOG(LOG_ERROR, "Invalid tid : %d", tid);
        return NULL;
    }

    return (Thread *)ListGet(gThreadsList, tid);
}