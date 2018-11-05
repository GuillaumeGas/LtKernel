#include "ext2.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#define MBR_RESERVED_SIZE 1024
#define INODE_NB_DIRECT_PTR 12
#define INODE_SINGLY_INDIRECT_PTR_INDEX 12
#define INODE_DOUBLY_INDIRECT_PTR_INDEX 13
#define INODE_TRIPLY_INDIRECT_PTR_INDEX 14

// https://wiki.osdev.org/Ext2

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

	//kprint("disk size : %d\n", disk->blockSize);
	//kprint("groups : %d\n", disk->groups);
	//kprint("blocksCount : %d\n", disk->superBlock->blocksCount);
	//kprint("blocksPerGroup : %d\n", disk->superBlock->blocksPerGroup);
	//kprint("errors : %d\n", disk->superBlock->errors);

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

	return NULL;
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

	// https://wiki.osdev.org/Ext2#Inodes
	inodeGroupIndex = (num - 1) / disk->superBlock->inodesPerGroup;
	inodeIndex = (num - 1) % disk->superBlock->inodesPerGroup;

	offset = (disk->groupDec[inodeGroupIndex].inodeTable * disk->blockSize) + (inodeIndex * disk->superBlock->inodeSize);
/*
	kprint("offset : %d\n", offset);
	kprint("inodeGroupIndex : %d\n", inodeGroupIndex);
	kprint("disk->groupDec[inodeGroupIndex].inodeTable : %d\n", disk->groupDec[inodeGroupIndex].inodeTable);
	kprint("disk->blockSize : %d\n", disk->blockSize);
	kprint("inodeIndex : %d\n", inodeIndex);
	kprint("disk->superBlock->inodeSize : %d\n", disk->superBlock->inodeSize);
*/
	ret = AtaRead(disk->device, inode, offset, disk->superBlock->inodeSize);

	if (ret < 0)
	{
		kprint("ext2.c!Ext2ReadInode() : AtaRead() returned %d\n", ret);
		goto clean;
	}

	kprint("size : %d\n", inode->size);
	inode->size = 0x1000;

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

Ext2File * Ext2ReadFile(Ext2Disk * disk, Ext2Inode * inode)
{
	char * file = NULL;
	Ext2File * res = NULL;
	char * block = NULL;
	u32 * singlyBlock = NULL;
	u32 * doublyBlock = NULL;
	u32 fileSize = 0;
	unsigned int fileOffset = 0, size = 0;

	fileSize = inode->size;

	if (fileSize <= 0)
		return NULL;

	kprint("try ?\n");
	file = (char *)kmalloc(1000);

	kprint("test file size : %d\n", fileSize);

	if (file == NULL)
	{
		kprint("ext2.c!Ext2ReadFile() : can't allocate memory for file, kmalloc() returned NULL\n");
		return NULL;
	}

	block = (char *)kmalloc(disk->blockSize);
	if (block == NULL)
	{
		kprint("ext2.c!Ext2ReadFile() : can't allocate memory for block buffer, kmalloc() returned NULL\n");
		goto clean;
	}

	// Direct blocks ptr
	for (int i = 0; i < INODE_NB_DIRECT_PTR && inode->block[i]; i++)
	{
		unsigned long offset = inode->block[i] * disk->blockSize;
		int ret = AtaRead(disk->device, block, offset, disk->blockSize);

		if (ret < 0)
		{
			kprint("ext2.c!Ext2ReadFile() : AtaRead() failed for direct block ptr with code : %d\n", ret);
			goto clean;
		}

		size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
		MmCopy((u8 *)block, (u8 *)(file + fileOffset), size);
		fileOffset += size;
		fileSize -= size;
	}

	// Singly Indirect block ptr
	if (inode->block[INODE_SINGLY_INDIRECT_PTR_INDEX])
	{
		singlyBlock = (u32 *)kmalloc(disk->blockSize);
		if (singlyBlock == NULL)
		{
			kprint("ext2.c!Ext2ReadFile() : can't allocate memory for singly block buffer, kmalloc() returned NULL\n");
			goto clean;
		}

		unsigned long offset = inode->block[INODE_SINGLY_INDIRECT_PTR_INDEX] * disk->blockSize;
		int ret = AtaRead(disk->device, singlyBlock, offset, disk->blockSize);

		if (ret < 0)
		{
			kprint("ext2.c!Ext2ReadFile() : AtaRead() failed for singly indirect block with code : %d\n", ret);
			goto clean;
		}

		for (int i = 0; i < disk->blockSize / sizeof(u32) && singlyBlock[i]; i++)
		{
			offset = singlyBlock[i] * disk->blockSize;
			int ret = AtaRead(disk->device, block, offset, disk->blockSize);

			if (ret < 0)
			{
				kprint("ext2.c!Ext2ReadFile() : AtaRead() failed for data in sinply indirect block ptr with code : %d\n", ret);
				goto clean;
			}

			size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
			MmCopy((u8 *)block, (u8 *)(file + fileOffset), size);
			fileOffset += size;
			fileSize -= size;
		}
	}

	// Double indirect block ptr
	if (inode->block[INODE_DOUBLY_INDIRECT_PTR_INDEX])
	{
		if (singlyBlock == NULL)
		{
			singlyBlock = (u32 *)kmalloc(disk->blockSize);
			if (singlyBlock == NULL)
			{
				kprint("ext2.c!Ext2ReadFile() : can't allocate memory for singly block buffer, kmalloc() returned NULL\n");
				goto clean;
			}
		}

		doublyBlock = (u32 *)kmalloc(disk->blockSize);
		if (doublyBlock == NULL)
		{
			kprint("ext2.c!Ext2ReadFile() : can't allocate memory for doubly block buffer, kmalloc() returned NULL\n");
			goto clean;
		}

		unsigned long offset = inode->block[INODE_DOUBLY_INDIRECT_PTR_INDEX] * disk->blockSize;
		int ret = AtaRead(disk->device, singlyBlock, offset, disk->blockSize);

		if (ret < 0)
		{
			kprint("ext2.c!Ext2ReadFile() : AtaRead() failed for singly indirect block with code : %d\n", ret);
			goto clean;
		}

		for (int i = 0; i < disk->blockSize / sizeof(u32) && singlyBlock[i]; i++)
		{
			offset = singlyBlock[i] * disk->blockSize;
			ret = AtaRead(disk->device, doublyBlock, offset, disk->blockSize);

			if (ret < 0)
			{
				kprint("ext2.c!Ext2ReadFile() : AtaRead() failed for doubly indirect block ptr with code : %d\n", ret);
				goto clean;
			}

			for (int j = 0; j < disk->blockSize / sizeof(u32) && doublyBlock[j]; j++)
			{
				offset = doublyBlock[i] * disk->blockSize;
				int ret = AtaRead(disk->device, block, offset, disk->blockSize);

				if (ret < 0)
				{
					kprint("ext2.c!Ext2ReadFile() : AtaRead() failed for data in double direct block ptr with code : %d\n", ret);
					goto clean;
				}

				size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
				MmCopy((u8 *)block, (u8 *)(file + fileOffset), size);
				fileOffset += size;
				fileSize -= size;
			}
		}
	}

	// TODO : triply indirect, mais faut faire du ménage, trop l'bordel

	res = (Ext2File *)file;

clean:
	if (file != NULL)
	{
		kfree(file);
		file = NULL;
	}

	if (block != NULL)
	{
		kfree(block);
		block = NULL;
	}

	if (singlyBlock != NULL)
	{
		kfree(singlyBlock);
		singlyBlock = NULL;
	}

	if (doublyBlock != NULL)
	{
		kfree(doublyBlock);
		doublyBlock = NULL;
	}

	return res;
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