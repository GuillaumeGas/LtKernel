#include "ext2.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#define MBR_RESERVED_SIZE 1024

typedef Ext2Disk Disk;
typedef AtaDevice Device;

static Ext2SuperBlock * ReadSuperBlock(Device * device);
static Ext2GroupDesc * ReadGroupDesc(Disk * disk);

Disk * Ext2ReadDiskOnDevice(AtaDevice * device) 
{
	Disk * disk = NULL;
	Ext2SuperBlock * superBlock = NULL;
	Ext2GroupDesc * groupDesc = NULL;
	u32 i = 0, j = 0;

	if (device == NULL)
	{
		kprint("ext2.c!Ext2ReadDiskOnDevice() : invalid device parameter\n");
		return NULL;
	}

	disk = (Disk *)kmalloc(sizeof(Disk));
	if (disk == NULL)
	{
		kprint("ext2.c!Ext2ReadDiskOnDevice() : kmalloc() returned NULL !\n");
		return NULL;
	}

	superBlock = ReadSuperBlock(device);
	if (superBlock == NULL)
	{
		kprint("ext2.c!Ext2ReadDiskOnDevice() : ReadSuperBlock() returned NULL !\n");
		goto clean;
	}

	disk->device = device;
	disk->superBlock = superBlock;
	disk->blockSize = 1024 << superBlock->logBlockSize;

	i = (superBlock->blocksCount / superBlock->blocksPerGroup)
		+ ((superBlock->blocksCount % superBlock->blocksPerGroup) ? 1 : 0);
	j = (superBlock->inodesCount / superBlock->inodesPerGroup)
		+ ((superBlock->inodesCount % superBlock->inodesPerGroup) ? 1 : 0);

	disk->groups = (i > j ? i : j);

	groupDesc = ReadGroupDesc(disk);
	if (groupDesc == NULL)
	{
		kprint("ext2.c!Ext2ReadDiskOnDevice() : ReadGroupDesc() returned NULL !\n");
		goto clean;
	}

	disk->groupDec = groupDesc;

	return disk;

clean:
	if (disk != NULL)
	{
		kfree(disk);
		disk = NULL;
	}

	if (superBlock != NULL)
	{
		kfree(superBlock);
		superBlock = NULL;
	}
}

Ext2Inode * Ext2ReadInode(Ext2Disk * disk, int num)
{
	Ext2Inode * inode = NULL;
	int ret = 0;
	int inodeGroupIndex = 0;
	int inodeIndex = 0;
	int offset = 0;

	if (num < 1)
	{
		kprint("ext2.c!Ext2ReadInode() : inode number must be > 0 !\n");
		return NULL;
	}

	inode = (Ext2Inode *)kmalloc(sizeof(Ext2Inode));
	if (inode == NULL)
	{
		kprint("ext2.c!Ext2ReadInode() : kmalloc() returned NULL\n");
		return NULL;
	}

	inodeGroupIndex = (num - 1) / disk->superBlock->inodesPerGroup;
	inodeIndex = (num - 1) % disk->superBlock->inodesPerGroup;

	offset = (disk->groupDec[inodeGroupIndex].inodeTable * disk->blockSize) + (inodeIndex * disk->superBlock->inodeSize);

	ret = AtaRead(disk->device, inode, offset, disk->superBlock->inodeSize);

	if (ret < 0)
	{
		kprint("ext2.c!Ext2ReadInode() : AtaRead() returned %d\n", ret);
		goto clean;
	}

	return inode;

clean:
	if (inode != NULL)
	{
		kfree(inode);
		inode = NULL;
	}

	return NULL;
}

static Ext2SuperBlock * ReadSuperBlock(Device * device)
{
	Ext2SuperBlock * superBlock = NULL;
	const unsigned int offset = MBR_RESERVED_SIZE;
	int ret = 0;

	superBlock = (Ext2SuperBlock *)kmalloc(sizeof(Ext2SuperBlock));
	if (superBlock == NULL)
	{
		kprint("ext2.c!ReadSuperBlock() : kmalloc() returned NULL\n");
		return NULL;
	}

	ret = AtaRead(device, superBlock, sizeof(Ext2SuperBlock), offset);

	if (ret < 0)
	{
		kprint("ext2.c!ReadSuperBlock() : AtaRead() returned %d\n", ret);
		goto clean;
	}

	return superBlock;

clean:
	if (superBlock != NULL)
	{
		kfree(superBlock);
		superBlock = NULL;
	}

	return NULL;
}

static Ext2GroupDesc * ReadGroupDesc(Disk * disk)
{
	Ext2GroupDesc * groupDesc = NULL;
	int ret = 0;
	int size = disk->groups * sizeof(Ext2GroupDesc);
	unsigned long offset = (disk->blockSize == MBR_RESERVED_SIZE ? 2048 : disk->blockSize);

	groupDesc = (Ext2GroupDesc *)kmalloc(size);
	if (groupDesc == NULL)
	{
		kprint("ext2.c!ReadGroupDesc() : kmalloc() returned NULL\n");
		return NULL;
	}

	ret = AtaRead(disk->device, groupDesc, offset, (unsigned long)size);

	if (ret < 0)
	{
		kprint("ext2.c!ReadGroupDesc() : AtaRead() returned %d\n", ret);
		goto clean;
	}

	return groupDesc;

clean:
	if (groupDesc != NULL)
	{
		kfree(groupDesc);
		groupDesc = NULL;
	}

	return NULL;
}

void Ext2FreeDisk(Ext2Disk * disk)
{
	if (disk == NULL)
		return;

	if (disk->groupDec != NULL)
	{
		kfree(disk->groupDec);
		disk->groupDec = NULL;
	}

	if (disk->superBlock != NULL)
	{
		kfree(disk->superBlock);
		disk->superBlock = NULL;
	}

	kfree(disk);
	disk = NULL;
}