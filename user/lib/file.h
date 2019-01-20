#pragma once

typedef void * Handle;

struct DirEntry
{
	Handle handle;
	int isDirectory;
	char name[256];
} typedef DirEntry;

int OpenDir(const char * dirPath, Handle * dirHandle);
int ReadDir(const Handle fileHandle, DirEntry * dirEntry);