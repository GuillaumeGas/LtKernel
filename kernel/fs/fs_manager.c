#define __FS__
#include "fs_manager.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/fs/elf.h>

KeStatus FsInit(AtaDevice * device)
{
    KeStatus status = Ext2ReadDiskOnDevice(device, &gExt2Disk);
	if (FAILED(status) || gExt2Disk == NULL)
	{
		KLOG(LOG_ERROR, "File system intialization failed with status %d", status);
        return status;
	}

	//Ext2Inode * inode = Ext2ReadInode(gExt2Disk, 12);
	//if (inode == NULL)
	//{
	//	kprint("  Failed to retrieve inode 1 !\n");
	//}
	//else
	//{
	//	ElfHeader * file = (ElfHeader *)Ext2ReadFile(gExt2Disk, inode);
	//	if (file == NULL)
	//	{
	//		kprint("  Failed to read file !\n");
	//	}
	//	else
	//	{
	//		if (!ElfCheckIdent(file))
	//		{
	//			kprint("  Not a Elf file !\n");
	//		}
	//		else
	//		{
	//			kprint("  Bingo !\n");
	//		}
	//		kfree(file);
	//	}
	//	kfree(inode);
	//}
    return STATUS_SUCCESS;
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}