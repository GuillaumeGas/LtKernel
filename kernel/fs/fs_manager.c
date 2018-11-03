#define __FS__
#include "fs_manager.h"

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

		//Ext2Inode * inode = Ext2ReadInode(gExt2Disk, 1);
		//if (inode == NULL)
		//{
		//	kprint("  Failed to retrieve inode 1 !\n");
		//}
		//else
		//{
		//	kprint("  Inode uid : %d\n", inode->uid);
		//	kprint("  Inode size : %d\n", inode->size);
		//}
	}
}

void FsCleanCallback()
{
	Ext2FreeDisk(gExt2Disk);
}