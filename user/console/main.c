#include <stdio.h>
#include <stdlib.h>
#include <syscalls.h>
#include <file.h>

#define MAX_BUFFER_SIZE 255

#define CMD_PROCESS_LIST "plist"


void main()
{
	char begin[] = " > ";
	char buffer[MAX_BUFFER_SIZE];

    MmSet(buffer,'\0', MAX_BUFFER_SIZE);

	Handle fileHandle = NULL;
	int ret = OpenDir("/", &fileHandle);

	if (ret == 0)
	{
		Print("Success !");
	}
	else
	{
		Print("Echec !");
	}

	//while (1)
	//{
	//	Print(begin);
	//	Scan(buffer);

	//	if (StrCmp(buffer, CMD_PROCESS_LIST) == 0)
	//		ListProcess();
	//}

        while (1);

	// TODO : exit();
}
