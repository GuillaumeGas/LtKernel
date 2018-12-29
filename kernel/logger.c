#define __LOGGER__
#include "logger.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/drivers/screen.h>

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

static void _KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, va_list args)
{
	switch (level)
	{
	case LOG_INFO:
		break;
	case LOG_ERROR:
		ScSetColor(RED);
		break;
	case LOG_WARNING:
		ScSetColor(MAGENTA);
		break;
	case LOG_DEBUG:
		ScSetColor(CYAN);
		break;
	}
	if (level != LOG_INFO)
	{
		kprint("[%s] %s (%d) : ", moduleName, fileName, lineNumber);
	}
	else
	{
		kprint("[%s] ", moduleName);
	}
	kprintEx(format, args);
	kprint("\n");
	ScSetColor(WHITE);
}

void KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	_KernelLogger(fileName, lineNumber, moduleName, level, format, args);
	va_end(args);
}