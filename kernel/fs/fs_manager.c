#define __FS__
#include "fs_manager.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)
#include <kernel/debug/debug.h>

#include <kernel/fs/elf.h>

KeStatus FsInit(AtaDevice * device)
{
    KeStatus status = Ext2ReadDiskOnDevice(device, &gExt2Disk);
	if (FAILED(status) || gExt2Disk == NULL)
	{
		KLOG(LOG_ERROR, "File system intialization failed with status %d", status);
        return status;
	}

	Ext2Inode * inode = NULL;
	int inodeNumber = 12;
	status = Ext2ReadInode(gExt2Disk, inodeNumber, &inode);
	if (FAILED(status))
	{
		KLOG(LOG_ERROR, "Failed to retrieve inode %d !", inodeNumber);
	}
	else
	{
		ElfHeader * file = NULL;
		status = Ext2ReadFile(gExt2Disk, inode, (Ext2File *)&file);
		if (FAILED(status))
		{
			KLOG(LOG_ERROR, "Failed to read file !");
		}
		else
		{
			__debugbreak();
			if (!ElfCheckIdent(file))
			{
				KLOG(LOG_ERROR, "Not a Elf file !");
			}
			else
			{
				KLOG(LOG_DEBUG, "Bingo !");
			}
			kfree(file);
		}
		kfree(inode);
	}
    return STATUS_SUCCESS;
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}