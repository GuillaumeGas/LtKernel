#define __FS__
#include "fs_manager.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)
#include <kernel/debug/debug.h>

#include "elf.h"
#include "file.h"

static KeStatus InitRoot();

KeStatus FsInit(AtaDevice * device)
{
    KeStatus status = Ext2ReadDiskOnDevice(device, &gExt2Disk);
	if (FAILED(status) || gExt2Disk == NULL)
	{
		KLOG(LOG_ERROR, "File system intialization failed with status %d", status);
        return status;
	}

	status = InitRoot();
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "InitRootFile() failed with status %d", status);
		return status;
	}

    return STATUS_SUCCESS;
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}

void FreeFile(File * file)
{
	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
        return;
	}

    // TODO : compléter nettoyage...

	kfree(file);
}

static KeStatus InitRoot()
{
	KeStatus status = STATUS_FAILURE;
	Ext2Inode * inode = NULL;

	if (gExt2Disk == NULL)
	{
		KLOG(LOG_ERROR, "Ext2 file system is not initialized !");
		return STATUS_UNEXPECTED;
	}

	status = Ext2ReadInode(gExt2Disk, EXT2_ROOT_INODE_NUMBER, &inode);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
		goto clean;
	}

	status = CreateFile(gExt2Disk, inode, EXT2_ROOT_INODE_NUMBER, &gRootFile);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "CreateFile() failed with code %d", status);
		goto clean;
	}

	status = InitRootFile(gRootFile);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "InitRootFile() failed with code %d", status);
		goto clean;
	}

	status = BrowseAndCacheDirectory(gRootFile);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "BrowseAndCacheDirectory() failed with code %d", status);
		goto clean;
	}

	PrintDirectory(gRootFile);

	status = STATUS_SUCCESS;

clean:
	return status;
}