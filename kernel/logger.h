#pragma once

#include <stdarg.h>

#define KLOGGER(kmodule, level, format, ...) KernelLogger(__FILE__, __LINE__, kmodule, level, format, ##__VA_ARGS__)

enum LogType { LOG_SCREEN = 1, LOG_SERIAL = 2 } typedef LogType;
enum LogLevel { LOG_INFO, LOG_ERROR, LOG_WARNING, LOG_DEBUG } typedef LogLevel;

void KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, ...);

void LoggerInit(const LogType logType);
void LoggerSetType(const LogType logType);

#ifdef __LOGGER__
LogType g_logType;
#else
extern LogType g_logType;
#endif
