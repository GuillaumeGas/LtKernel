#include <stdio.h>
#include <stdlib.h>
#include <syscalls.h>
#include <file.h>

#define MAX_BUFFER_SIZE 255

#define CMD_PROCESS_LIST "plist"
#define CMD_LS "ls"

void Ls()
{
	Handle fileHandle = NULL;
	int ret = OpenDir("/", &fileHandle);

	if (ret == 0)
	{
		DirEntry dirEntry;
		while ((ret = ReadDir(fileHandle, &dirEntry)) != 0)
		{
			Print(dirEntry.name);
			Print("\n");
		}
	}
	else
	{
		Print("Echec !");
	}
}

void main()
{
	char begin[] = " > ";
	char buffer[MAX_BUFFER_SIZE];

    MmSet(buffer,'\0', MAX_BUFFER_SIZE);
    Print(begin);
    Scan(buffer);

	while (1)
	{
		//Print(begin);
		//Scan(buffer);

		//if (StrCmp(buffer, CMD_PROCESS_LIST) == 0)
		//	ListProcess();
		//else if (StrCmp(buffer, CMD_LS) == 0)
		//	Ls();
	}

    while (1);

	// TODO : exit();
}
