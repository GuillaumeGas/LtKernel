#include "file.h"
#include "syscalls.h"

int OpenDir(const char * dirPath, Handle * dirHandle)
{
	return _openDir(dirPath, dirHandle);
}

int ReadDir(const Handle fileHandle, DirEntry * dirEntry)
{
	return _readDir(fileHandle, dirEntry);
}

int GetProcessDirectory(Handle * handle)
{
    return _getProcessDirectory(handle);
}

int SetCurrentDirectory(const Handle handle)
{
    return _setCurrentDirectory(handle);
}