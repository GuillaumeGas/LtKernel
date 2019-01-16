#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 255

#define CMD_PROCESS_LIST "plist"

void main()
{
	char begin[] = "> ";
	char buffer[MAX_BUFFER_SIZE];

	MmSet(buffer,'\0', MAX_BUFFER_SIZE);

	while (1)
	{
		Print(begin);
		Scan(buffer);

		if (StrCmp(buffer, CMD_PROCESS_LIST) == 0)
			ListProcess();
	}

	// TODO : exit();
}
