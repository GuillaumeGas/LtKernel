#include "file.h"
#include "syscalls.h"

int OpenDir(const char * dirPath, Handle * dirHandle)
{
	return _openDir(dirPath, dirHandle);
}