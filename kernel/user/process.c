#include "process.h"

#include <kernel/user/process_manager.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

Process * GetCurrentProcess()
{
    return gCurrentProcess;
}

Process * GetProcessFromPid(int pid)
{
	if (pid < 0)
	{
		KLOG(LOG_ERROR, "Invalid pid : %d", pid);
		return NULL;
	}

	return (Process *)ListGet(gProcessList, pid);
}