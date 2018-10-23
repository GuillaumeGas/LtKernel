#pragma once

enum LogType { LOG_SCREEN = 1, LOG_SERIAL = 2 };

typedef enum LogType LogType;

void LoggerInit(const LogType logType);
void LoggerSetType(const LogType logType);

#ifdef __LOGGER__
LogType g_logType;
#else
extern LogType g_logType;
#endif
