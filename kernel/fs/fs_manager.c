#define __FS__
#include "fs_manager.h"

#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/fs/elf.h>

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

		Ext2Inode * inode = Ext2ReadInode(gExt2Disk, 12);
		if (inode == NULL)
		{
			kprint("  Failed to retrieve inode 1 !\n");
		}
		else
		{
			ElfHeader * file = (ElfHeader *)Ext2ReadFile(gExt2Disk, inode);
			if (file == NULL)
			{
				kprint("  Failed to read file !\n");
			}
			else
			{
				if (!ElfCheckIdent(file))
				{
					kprint("  Not a Elf file !\n");
				}
				else
				{
					kprint("  Bingo !\n");
				}
				kfree(file);
			}
			kfree(inode);
		}
	}
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}