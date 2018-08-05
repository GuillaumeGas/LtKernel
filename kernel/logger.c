#define __LOGGER__
#include "logger.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

void init_logger(const LogType logType)
{
	set_logger(logType);

	kprint("[LOGGER] Initialized with :");
	if (FlagOn(g_logType, LOG_SCREEN))
		kprint(" LOG_SCREEN");
	if (FlagOn(g_logType, LOG_SERIAL))
		kprint(" LOG_SERIAL");
	kprint("\n");
}

void set_logger(const LogType logType)
{
	g_logType = logType;
}
