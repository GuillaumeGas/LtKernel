#include "handle.h"
#include "stdlib.h"
#include "kmalloc.h"

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("LIB", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/debug/debug.h>

static int * s_HandleCount = 0;

KeStatus CreateFileHandle(File * file, FileHandle ** fileHandle)
{
	KeStatus status = STATUS_FAILURE;
	FileHandle * localHandle = NULL;

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (fileHandle == NULL)
	{
		KLOG(LOG_ERROR, "Invalid fileHandle parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (IsDirectory(file))
	{
		KLOG(LOG_ERROR, "Can't create a file handle from a directory");
		return STATUS_NOT_A_FILE;
	}

	localHandle = (FileHandle *)kmalloc(sizeof(FileHandle));
	if (localHandle == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(FileHandle));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	localHandle->handle = s_HandleCount++;
	localHandle->file = file;
	localHandle->cursor = 0;

	*fileHandle = localHandle;
	localHandle = NULL;

	status = STATUS_SUCCESS;

clean:
	return status;
}

KeStatus CreateDirHandle(File * dir, DirHandle ** dirHandle)
{
	KeStatus status = STATUS_FAILURE;
	DirHandle * localHandle = NULL;

	if (dir == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dir parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (dirHandle == NULL)
	{
		KLOG(LOG_ERROR, "Invalid dirHandle parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (IsDirectory(dir) == FALSE)
	{
		KLOG(LOG_ERROR, "Can't create a directory handle from a file");
		return STATUS_NOT_A_DIRECTORY;
	}

	localHandle = (DirHandle *)kmalloc(sizeof(DirHandle));
	if (localHandle == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(DirHandle));
		status = STATUS_ALLOC_FAILED;
		goto clean;
	}

	KLOG(LOG_DEBUG, "Creating dir handle for %s", dir->name);

	localHandle->handle = s_HandleCount++;
	localHandle->dir = dir;
	localHandle->cursor = NULL;

	*dirHandle = localHandle;
	localHandle = NULL;

	status = STATUS_SUCCESS;

clean:
	return status;
}

void FreeFileHandle(FileHandle * fileHandle)
{
	if (fileHandle == NULL)
	{
		KLOG(LOG_ERROR, "Invalid fileHandle parameter");
		return;
	}

	//if (fileHandle->file != NULL)
	//{
	//	FreeFile(fileHandle->file);
	//	fileHandle->file = NULL;
	//}

	kfree(fileHandle);
	fileHandle = NULL;
}

void FreeDirHandle(DirHandle * dirHandle)
{
	if (dirHandle == NULL)
	{
		KLOG(LOG_ERROR, "Invalid fileHandle parameter");
		return;
	}

	//if (dirHandle->dir != NULL)
	//{
	//	FreeFile(dirHandle->dir);
	//	dirHandle->dir = NULL;
	//}

	kfree(dirHandle);
	dirHandle = NULL;
}