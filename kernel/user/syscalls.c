#include "syscalls.h"

#include <kernel/drivers/clock.h>
#include <kernel/drivers/screen.h>

#include <kernel/user/process.h>
#include <kernel/user/thread.h>
#include <kernel/user/console.h>
#include <kernel/user/process_manager.h>

#include <kernel/debug/debug.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

#include <kernel/fs/fs_manager.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("USER", LOG_LEVEL, format, ##__VA_ARGS__)

static KeStatus SysPrint(const char * str, int * ret);
static KeStatus SysScanf(char * buffer, int * ret);
static KeStatus SysExec(void * funAddr, int * ret);
static KeStatus SysWait(int pid, int * ret);
static KeStatus SysExit(int * ret);

/* FS syscalls */
static KeStatus SysOpenDir(const char * dirPath, Handle * dirHandle, int * ret);
static KeStatus SysOpenDirHandle(const Handle dirHandle, int * ret);
static KeStatus SysReadDir(const Handle dirHandle, DirEntry * dirEntry, int * ret);
static KeStatus SysCloseDir(const Handle dirHandle, int * ret);
static KeStatus SysRewindDir(const Handle dirHandle, int * ret);

//static KeStatus SysOpenFile(const char * filePath, Handle * dirHandle, int * ret);
//static KeStatus SysReadFile(const Handle dirHandle, FileCursorType beginType, uint bytes, char * buffer, int * ret);
//static KeStatus SysCloseFile(const Handle dirHandle, int * ret);
//static KeStatus SysRewindFile(const Handle dirHandle, int * ret);

static KeStatus SysDebugListProcess(int * ret);

enum SyscallId
{
    SYSCALL_PRINT = 1,
    SYSCALL_SCANF = 2,
    SYSCALL_EXEC  = 3,
	SYSCALL_WAIT  = 4,
    SYSCALL_EXIT  = 5,

	SYSCALL_OPEN_DIR = 6,
	SYSCALL_OPEN_DIR_HANDLE = 7,
	SYSCALL_READ_DIR = 8,
	SYSCALL_CLOSE_DIR = 9,
	SYSCALL_REWIND_DIR = 10,

    SYSCALL_DEBUG_LIST_PROCESS   = 255,
    SYSCALL_DEBUG_DUMP_REGISTERS = 256,
};

void SyscallHandler(int syscallNumber, InterruptContext * context)
{
	int ret = 0;
	KeStatus status = STATUS_FAILURE;

    switch (syscallNumber)
    {
        case SYSCALL_PRINT:
			status = SysPrint((char *)context->ebx, &ret);
            break;
        case SYSCALL_SCANF:
			status = SysScanf((char *)context->ebx, &ret);
            break;
        case SYSCALL_EXEC:
			status = SysExec((void *)context->ebx, &ret);
            break;
		case SYSCALL_WAIT:
			status = SysWait((int)context->ebx, &ret);
			break;
        case SYSCALL_EXIT:
			status = SysExit(&ret);
            break;

		case SYSCALL_OPEN_DIR:
			status = SysOpenDir((const char *)context->ebx, (Handle *)context->ecx, &ret);
			break;
		case SYSCALL_OPEN_DIR_HANDLE:
			status = SysOpenDirHandle((const Handle)context->ebx, &ret);
			break;
		case SYSCALL_READ_DIR:
			status = SysReadDir((const Handle)context->ebx, (DirEntry *)context->ecx, &ret);
			break;
		case SYSCALL_CLOSE_DIR:
			status = SysCloseDir((const Handle)context->ebx, &ret);
			break;
		case SYSCALL_REWIND_DIR:
			status = SysRewindDir((const Handle)context->ebx, &ret);
			break;

        case SYSCALL_DEBUG_LIST_PROCESS:
			status = SysDebugListProcess(&ret);
            break;
        default:
            KLOG(LOG_ERROR, "Unknown system call !");
    }
	context->eax = ret;

	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "An error occured while executing syscall");
	}
}

static KeStatus SysPrint(const char * str, int * ret)
{
	if (str == NULL)
	{
		KLOG(LOG_ERROR, "Invalid str parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    kprint(str);
    *ret = 0;

	return STATUS_SUCCESS;
}

static KeStatus SysScanf(char * buffer, int * ret)
{
	if (buffer == NULL)
	{
		KLOG(LOG_ERROR, "Invalid buffer parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    Thread * currentThread = GetCurrentThread();
	if (currentThread == NULL)
	{
		KLOG(LOG_ERROR, "GetCurrentThread() returned NULL");
		*ret = -1; // TODO : definir quelques codes d'erreur de retour, types qui seront utilisés dans les libs userland
		return STATUS_PROCESS_NOT_FOUND;
	}

	ScEnableCursor();

    if (CnslIsAvailable())
    {
        CnslSetOwnerThread(currentThread);
    }
	else
	{
		Thread * currentCnslOwner = CnslGetOwnerThread();

		if (currentCnslOwner != currentThread)
		{
			while (!CnslIsAvailable());

			// TODO : Add lock/mutex whatever ?
			CnslSetOwnerThread(currentThread);
		}
	}

	while (!currentThread->console.readyToBeFlushed);

	ScDisableCursor();

	MmCopy((u8 *)currentThread->console.consoleBuffer, (u8 *)buffer, currentThread->console.bufferIndex + 1);
	buffer[currentThread->console.bufferIndex] = '\0';

	CnslFreeOwnerThread();

	currentThread->console.bufferIndex = 0;
	currentThread->console.readyToBeFlushed = FALSE;

	*ret = 0;

	return STATUS_SUCCESS;
}

static KeStatus SysExec(void * funAddr, int * ret)
{
	KeStatus status = STATUS_FAILURE;

    if (funAddr == NULL)
    {
        KLOG(LOG_ERROR, "Invalid funAddr parameter");
        return STATUS_NULL_PARAMETER;
    }

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

 //   Process * currentProcess = GetCurrentProcess();
 //   if (currentProcess == NULL)
 //   {
	//	KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL");
	//	status = STATUS_PROCESS_NOT_FOUND;
	//	*ret = -1;
	//	goto clean;
 //   }

	//int newProcPid = -1;
	//status = PmCreateProcess(funAddr, 500 /*tmp !*/, currentProcess, &newProcPid);
	//if (FAILED(status) || newProcPid == -1)
	//{
	//	KLOG(LOG_ERROR, "PmCreateProcess() failed with status : %d", status);
	//	*ret = -1;
	//	goto clean;
	//}
	/*Process * newProcess = GetProcessFromPid(newProcPid);*/
	
	// TODO, implémenter syscall_wait !
	//while (newProcess->state != PROCESS_STATE_DEAD);

	//*ret = newProcPid;

//clean:
	return status;
}

static KeStatus SysWait(int pid, int * ret)
{
	KeStatus status = STATUS_FAILURE;

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

	// On empêche d'attendre sur le processus system pour l'instant...
	if (pid == 0)
	{
		KLOG(LOG_DEBUG, "Can't wait on system process !");
		status = STATUS_INVALID_PARAMETER;
		goto clean;
	}

	Process * currentProc = GetCurrentProcess();
	Process * process = GetProcessFromPid(pid);

	if (currentProc == NULL)
	{
		KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL");
		status = STATUS_PROCESS_NOT_FOUND;
		goto clean;
	}

	if (currentProc->pid == pid)
	{
		KLOG(LOG_ERROR, "A process can't wait its own death !");
		status = STATUS_FAILURE;
		goto clean;
	}

	if (process == NULL)
	{
		KLOG(LOG_ERROR, "Can't find process with pid : %d", pid);
		status = STATUS_PROCESS_NOT_FOUND;
		goto clean;
	}

	while (process->state == PROCESS_STATE_RUNNING);

	status = STATUS_SUCCESS;
	*ret = 0;

clean:
	return status;
}

static KeStatus SysExit(int * ret)
{
	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    Process * currentProcess = GetCurrentProcess();
    if (currentProcess == NULL)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL !");
		return STATUS_PROCESS_NOT_FOUND;
    }

    currentProcess->state = PROCESS_STATE_DEAD;
	gNbProcess--;

	*ret = 0;
	return STATUS_SUCCESS;
}

static KeStatus SysOpenDir(const char * dirPath, Handle * dirHandle, int * ret)
{
	KeStatus status = STATUS_FAILURE;

	if (dirPath == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dirPath parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (dirHandle == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dirHandle parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

	File * file = NULL;
	status = OpenFileFromName(dirPath, &file);
	if (FAILED(status))
	{
		// TODO : les erreurs syscall sont pas forcément des erreurs du noyau,
		// on devrait les logger autrement
		KLOG(LOG_WARNING, "OpenFileFromName() failed with code %d", status);
		status = STATUS_SUCCESS; // le syscall n'a pas vraiment échoué, le user est juste mauvais
		*ret = -1; // FILE_NOT_FOUND
		goto clean;
	}

	if (IsDirectory(file) == FALSE)
	{
		KLOG(LOG_WARNING, "%s is not a directory", dirPath);
		*ret = -2; // NOT_A_DIRECTORY
		goto clean;
	}

	DirHandle * localHandle = NULL;
	status = CreateDirHandle(file, &localHandle);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "CreateFileHandle() failed with code %d", status);
		*ret = -3; // INTERNAL_ERROR
		goto clean;
	}

	Process * currentProcess = GetCurrentProcess();
	if (currentProcess == NULL)
	{
		KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL");
		*ret = -3; // INTERNAL_ERROR
		return STATUS_PROCESS_NOT_FOUND;
	}

	ListPush(currentProcess->dirHandleList, localHandle);

	*dirHandle = localHandle->handle;
	dirHandle = NULL;

	status = STATUS_SUCCESS;

clean:
	return status;
}

static KeStatus SysOpenDirHandle(const Handle dirHandle, int * ret) 
{
	return STATUS_SUCCESS;
}

static KeStatus SysReadDir(const Handle handle, DirEntry * dirEntry, int * ret)
{
	KeStatus status = STATUS_FAILURE;
	DirHandle * dirHandle = NULL;
	Process * process = NULL;

	if (handle == NULL)
	{
		KLOG(LOG_ERROR, "Invalid handle parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (dirEntry == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dirEntry parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

	process = GetCurrentProcess();
	if (process == NULL)
	{
		KLOG(LOG_ERROR, "GetCurrentProcess() returned NULL");
		*ret = -3; // INTERNAL_ERROR
		status = STATUS_PROCESS_NOT_FOUND;
		goto clean;
	}

	status = GetDirFromHandle(handle, process, &dirHandle);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "GetDirFromHandle() failed with code %d", status);
		*ret = -4; // UNKNOWN HANDLE
		goto clean;
	}

	if (dirHandle->dir == NULL)
	{
		KLOG(LOG_ERROR, "dirHandle->dir is NULL !");
		status = STATUS_UNEXPECTED;
		goto clean;
	}

	if (dirHandle->cursor == NULL)
	{
		if (dirHandle->dir->leaf == NULL)
		{
			*ret = 0; // EMPTY DIRECTORY
			status = STATUS_SUCCESS;
			goto clean;
		}
		else
		{
			dirHandle->cursor = dirHandle->dir->leaf;
		}
	}
	else
	{
		dirHandle->cursor = dirHandle->cursor->next;
	}

	if (dirHandle->cursor == NULL)
	{
		*ret = 0; // EMPTY DIRECTORY
		status = STATUS_SUCCESS;
		goto clean;
	}

	if (IsDirectory(dirHandle->cursor))
	{
		DirHandle * subDirHandle = NULL;
		status = CreateDirHandle(dirHandle->cursor, &subDirHandle);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "CreateDirHandle() failed with code %d", status);
			*ret = -3; // INTERNAL ERROR
			goto clean;
		}

		ListPush(process->dirHandleList, subDirHandle);

		dirEntry->handle = subDirHandle->handle;
		dirEntry->isDirectory = TRUE;
	}
	else
	{
		FileHandle * fileHandle = NULL;
		status = CreateFileHandle(dirHandle->cursor, &fileHandle);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "CreateFileHandle() failed with code %d", status);
			*ret = -3; // INTERNAL ERROR
			goto clean;
		}

		ListPush(process->fileHandleList, fileHandle->handle);

		dirEntry->handle = fileHandle->handle;
		dirEntry->isDirectory = FALSE;
	}

	// TODO : StrCpySafe, copie de mem kernel vers mem user...
	StrCpy(dirHandle->cursor->name, dirEntry->name);

	status = STATUS_SUCCESS;
	*ret = 1;

clean:
	return status;
}

static KeStatus SysCloseDir(const Handle dirHandle, int * ret)
{
	return STATUS_SUCCESS;
}

static KeStatus SysRewindDir(const Handle dirHandle, int * ret)
{
	return STATUS_SUCCESS;
}

static KeStatus SysDebugListProcess(int * ret)
{
	if (ret == NULL)
	{
		KLOG(LOG_ERROR, "Invalid ret parameter");
		return STATUS_NULL_PARAMETER;
	}

    PmPrintProcessList();
	*ret = 0;
	return STATUS_SUCCESS;
}