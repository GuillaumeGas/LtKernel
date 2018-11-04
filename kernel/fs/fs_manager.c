#define __FS__
#include "fs_manager.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

void FsInit(AtaDevice * device)
{
	gExt2Disk = Ext2ReadDiskOnDevice(device);
	if (gExt2Disk == NULL)
	{
		kprint("[Kernel] Failed to initialize file system !\n");
	}
	else
	{
		kprint("[Kernel] Ext2 file system initialized\n");

		Ext2Inode * inode = Ext2ReadInode(gExt2Disk, 1);
		if (inode == NULL)
		{
			kprint("  Failed to retrieve inode 1 !\n");
		}
		else
		{
			kprint("  Inode uid : %d\n", inode->uid);
			kprint("  Inode size : %d\n", inode->size);

			File * file = (File *)Ext2ReadFile(gExt2Disk, inode);
			if (file == NULL)
			{
				kprint("  Failed to read file !\n");
			}
			else
			{
				kprint("  field1 : %d\n", file->field1);
				kprint("  filed2 : %d\n", file->field2);
			}

			kfree(file);
		}
	}
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}