#include <stdio.h>
#include <stdlib.h>
#include <file.h>

#define MAX_BUFFER_SIZE 255

#define CMD_PROCESS_LIST "plist"
#define CMD_LS "ls"
#define CMD_CD "cd"

static Handle s_currentDirHandle = NULL;

int CmdInit()
{
    int ret = GetProcessDirectory(&s_currentDirHandle);

    if (ret < 0)
    {
        Print("OpenDir failed !\n");
    }

    return ret;
}

void Ls()
{
	DirEntry dirEntry;
    int ret = 0;
	while ((ret = ReadDir(s_currentDirHandle, &dirEntry)) != 0)
	{
		Print(dirEntry.name);
		Print("\n");
	}
}

int Cd(const char * dirPath)
{
    Handle newDirHandle = NULL;

    if (dirPath == NULL)
    {
        Print("Empty directory name !\n");
        return -1;
    }

    int ret = OpenDir(dirPath, &newDirHandle);
    if (ret < 0)
    {
        Print("Cd failed (OpenFile) !\n");
    }

    ret = SetCurrentDirectory(newDirHandle);
    if (ret < 0)
    {
        Print("Cd failed (SetCurrentDirectory)\n");
    }

    s_currentDirHandle = newDirHandle;
    newDirHandle = NULL;

    return ret;
}

void main()
{
	char begin[] = " > ";
	char buffer[MAX_BUFFER_SIZE];
    int ret = 0;

    MmSet(buffer,'\0', MAX_BUFFER_SIZE);

    ret = CmdInit();

    if (ret < 0)
    {
        Print("Console initialization failed !");
        goto end;
    }

	while (1)
	{
		Print(begin);
		Scan(buffer);

		if (StrCmp(buffer, CMD_PROCESS_LIST) == 0)
			ListProcess();
		else if (StrCmp(buffer, CMD_LS) == 0)
			Ls();
        else
        {
            int indexOfSpace = FirstIndexOf(buffer, ' ');
            if (indexOfSpace > 0)
            {
                buffer[indexOfSpace] = '\0';

                if (StrCmp(buffer, CMD_CD) == 0 && buffer[indexOfSpace + 1] != '\0')
                    Cd(&buffer[indexOfSpace + 1]);

                buffer[indexOfSpace] = ' ';
            }
        }
	}

end:
    while (1);

	// TODO : exit();
}
