#include "stdio.h"
#include "stdlib.h"
#include "syscalls.h"

void Print(const char * str)
{
	if (str == NULL)
		return;

	_print(str);
}

void Scan(char * buffer)
{
	if (buffer == NULL)
		return;

	_scan(buffer);

	unsigned long size = StrLen(buffer);
	buffer[size - 1] = '\0';
}

void Exit()
{
	_exit();
}

void ListProcess()
{
	_listProcess();
}