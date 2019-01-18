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

KeStatus ReadFileFromInode(int inodeNumber, File ** file)
{
    Ext2Inode * inode = NULL;
    KeStatus status = STATUS_FAILURE;

	if (file == NULL)
	{
		KLOG(LOG_ERROR, "Invalid file parameter");
		return STATUS_INVALID_PARAMETER;
	}

    status = Ext2ReadInode(gExt2Disk, inodeNumber, &inode);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Failed to retrieve inode %d !", inodeNumber);
		goto clean;
    }
     
	status = Ext2ReadFile(gExt2Disk, inode, inodeNumber, file);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Failed to read file !");
		goto clean;
    }

	status = STATUS_SUCCESS;

clean:
	if (inode != NULL)
	{
		kfree(inode);
	}

	return status;
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

	if (gExt2Disk == NULL)
	{
		KLOG(LOG_ERROR, "Ext2 file system is not initialized !");
		return STATUS_UNEXPECTED;
	}

	status = ReadFileFromInode(EXT2_ROOT_INODE_NUMBER, &gRootFile);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "ReadFileFromInode() failed with code %d", status);
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