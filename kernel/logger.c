#define __LOGGER__
#include "logger.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

void LoggerInit(const LogType logType)
{
	LoggerSetType(logType);

	kprint("[LOGGER] Initialized with :");
	if (FlagOn(g_logType, LOG_SCREEN))
		kprint(" LOG_SCREEN");
	if (FlagOn(g_logType, LOG_SERIAL))
		kprint(" LOG_SERIAL");
	kprint("\n");
}

void LoggerSetType(const LogType logType)
{
	g_logType = logType;
}
