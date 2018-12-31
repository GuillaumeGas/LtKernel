#include "ext2.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

#include <kernel/logger.h>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("EXT2", LOG_LEVEL, format, ##__VA_ARGS__)

#define MBR_RESERVED_SIZE 1024
#define INODE_NB_DIRECT_PTR 12
#define INODE_SINGLY_INDIRECT_PTR_INDEX 12
#define INODE_DOUBLY_INDIRECT_PTR_INDEX 13
#define INODE_TRIPLY_INDIRECT_PTR_INDEX 14

// https://wiki.osdev.org/Ext2

typedef Ext2Disk Disk;
typedef AtaDevice Device;

static KeStatus ReadSuperBlock(Device * device, Ext2SuperBlock ** superBlock);
static KeStatus ReadGroupDesc(Ext2Disk * disk, Ext2GroupDesc ** groupDesc);

KeStatus Ext2ReadDiskOnDevice(AtaDevice * device, Ext2Disk ** disk)
{
	Ext2Disk * localDisk = NULL;
	Ext2SuperBlock * superBlock = NULL;
	Ext2GroupDesc * groupDesc = NULL;
    KeStatus status = STATUS_FAILURE;
	u32 i = 0, j = 0;

	if (device == NULL)
	{
		KLOG(LOG_ERROR, "Invalid device parameter");
        return STATUS_NULL_PARAMETER;
	}

    if (disk == NULL)
    {
        KLOG(LOG_ERROR, "Invalid disk parameter");
        return STATUS_NULL_PARAMETER;
    }

    *disk = NULL;

    localDisk = (Disk *)kmalloc(sizeof(Disk));
	if (localDisk == NULL)
	{
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Disk));
        status = STATUS_ALLOC_FAILED;
	}

	status = ReadSuperBlock(device, &superBlock);
	if (FAILED(status) || superBlock == NULL)
	{
		KLOG(LOG_ERROR, "ReadSuperBlock() failed with status %d", status);
		goto clean;
	}

    localDisk->device = device;
    localDisk->superBlock = superBlock;
    localDisk->blockSize = 1024 << superBlock->logBlockSize;

	i = (superBlock->blocksCount / superBlock->blocksPerGroup)
		+ ((superBlock->blocksCount % superBlock->blocksPerGroup) ? 1 : 0);
	j = (superBlock->inodesCount / superBlock->inodesPerGroup)
		+ ((superBlock->inodesCount % superBlock->inodesPerGroup) ? 1 : 0);

    localDisk->groups = (i > j ? i : j);

	status = ReadGroupDesc(localDisk, &groupDesc);
	if (groupDesc == NULL)
	{
		KLOG(LOG_ERROR, "ReadGroupDesc() failed with status %d", status);
		goto clean;
	}

    localDisk->groupDec = groupDesc;

    *disk = localDisk;
    localDisk = NULL;
    superBlock = NULL;

    status = STATUS_SUCCESS;

clean:
	if (localDisk != NULL)
	{
		kfree(localDisk);
		localDisk = NULL;
	}

	if (superBlock != NULL)
	{
		kfree(superBlock);
		superBlock = NULL;
	}

	return status;
}

KeStatus Ext2ReadInode(Ext2Disk * disk, int num, Ext2Inode ** inode)
{
	Ext2Inode * localInode = NULL;
	int ret = 0;
	int inodeGroupIndex = 0;
	int inodeIndex = 0;
	int offset = 0;
    KeStatus status = STATUS_FAILURE;

    if (inode == NULL)
    {
        KLOG(LOG_ERROR, "Invalid inode parameter");
        return STATUS_NULL_PARAMETER;
    }

    *inode = NULL;

	if (num < 1)
	{
		KLOG(LOG_ERROR, "Inode number must be > 0 !");
		return STATUS_INVALID_PARAMETER;
	}

    localInode = (Ext2Inode *)kmalloc(sizeof(Ext2Inode));
	if (localInode == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Ext2Inode));
        status = STATUS_ALLOC_FAILED;
	}

	// https://wiki.osdev.org/Ext2#Inodes
	inodeGroupIndex = (num - 1) / disk->superBlock->inodesPerGroup;
	inodeIndex = (num - 1) % disk->superBlock->inodesPerGroup;

	offset = disk->groupDec[inodeGroupIndex].inodeTable * disk->blockSize + inodeIndex * disk->superBlock->inodeSize;

	ret = AtaRead(disk->device, localInode, offset, disk->superBlock->inodeSize);

	if (ret < 0)
	{
		KLOG(LOG_ERROR, "AtaRead() returned %d", ret);
        status = STATUS_FAILURE;
		goto clean;
	}

    *inode = localInode;
    localInode = NULL;

    status = STATUS_SUCCESS;

clean:
	if (localInode != NULL)
	{
		kfree(localInode);
        localInode = NULL;
	}

	return status;
}

static KeStatus ReadSuperBlock(Device * device, Ext2SuperBlock ** superBlock)
{
	Ext2SuperBlock * localSuperBlock = NULL;
	const unsigned int offset = MBR_RESERVED_SIZE;
	int ret = 0;
    KeStatus status = STATUS_FAILURE;

    if (device == NULL)
    {
        KLOG(LOG_ERROR, "Invalid device parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (superBlock == NULL)
    {
        KLOG(LOG_ERROR, "Invalid superBlock parameter");
        return STATUS_NULL_PARAMETER;
    }

    *superBlock = NULL;

	localSuperBlock = (Ext2SuperBlock *)kmalloc(sizeof(Ext2SuperBlock));
	if (localSuperBlock == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Ext2SuperBlock));
        status = STATUS_ALLOC_FAILED;
        goto clean;
	}

	ret = AtaRead(device, localSuperBlock, sizeof(Ext2SuperBlock), offset);

	if (ret < 0)
	{
		KLOG(LOG_ERROR, "AtaRead() returned %d", ret);
        status = STATUS_FAILURE;
        goto clean;
	}

    *superBlock = localSuperBlock;
    localSuperBlock = NULL;

    status = STATUS_SUCCESS;

clean:
	if (localSuperBlock != NULL)
	{
		kfree(localSuperBlock);
        localSuperBlock = NULL;
	}

	return status;
}

static KeStatus ReadGroupDesc(Ext2Disk * disk, Ext2GroupDesc ** groupDesc)
{
	Ext2GroupDesc * localGroupDesc = NULL;
	int ret = 0;
	int size = disk->groups * sizeof(Ext2GroupDesc);
	unsigned long offset = (disk->blockSize == MBR_RESERVED_SIZE ? 2048 : disk->blockSize);
    KeStatus status = STATUS_FAILURE;

    if (disk == NULL)
    {
        KLOG(LOG_ERROR, "Invalid disk parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (groupDesc == NULL)
    {
        KLOG(LOG_ERROR, "Invalid groupDesc parameter");
        return STATUS_NULL_PARAMETER;
    }

    *groupDesc = NULL;

    localGroupDesc = (Ext2GroupDesc *)kmalloc(size);
	if (localGroupDesc == NULL)
	{
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
        status = STATUS_ALLOC_FAILED;
        goto clean;
	}

	ret = AtaRead(disk->device, localGroupDesc, offset, (unsigned long)size);

	if (ret < 0)
	{
        KLOG(LOG_ERROR, "AtaRead() returned %d", ret);
        status = STATUS_FAILURE;
        goto clean;
	}

    *groupDesc = localGroupDesc;
    localGroupDesc = NULL;

    status = STATUS_SUCCESS;

clean:
	if (localGroupDesc != NULL)
	{
		kfree(localGroupDesc);
        localGroupDesc = NULL;
	}

	return status;
}

KeStatus Ext2ReadFile(Ext2Disk * disk, Ext2Inode * inode, Ext2File ** file)
{
	char * localFile = NULL;
	char * block = NULL;
	u32 * singlyBlock = NULL;
	u32 * doublyBlock = NULL;
	u32 fileSize = 0;
	unsigned int fileOffset = 0, size = 0;
    KeStatus status = STATUS_FAILURE;

    if (disk == NULL)
    {
        KLOG(LOG_ERROR, "Invalid disk parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (inode == NULL)
    {
        KLOG(LOG_ERROR, "Invalid inode parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (file == NULL)
    {
        KLOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    *file = NULL;

	fileSize = inode->size;
    localFile = (char *)kmalloc(fileSize);

	if (localFile == NULL)
	{
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
        status = STATUS_ALLOC_FAILED;
        goto clean;
	}

	block = (char *)kmalloc(disk->blockSize);
	if (block == NULL)
	{
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
        status = STATUS_ALLOC_FAILED;
        goto clean;
	}

	// Direct blocks ptr
	for (int i = 0; i < INODE_NB_DIRECT_PTR && inode->block[i]; i++)
	{
		unsigned long offset = inode->block[i] * disk->blockSize;
		int ret = AtaRead(disk->device, block, offset, disk->blockSize);

		if (ret < 0)
		{
            KLOG(LOG_ERROR, "AtaRead() failed for direct block ptr (returned %d)", ret);
            status = STATUS_FAILURE;
            goto clean;
		}

		size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
		MmCopy((u8 *)block, (u8 *)(localFile + fileOffset), size);

		fileOffset += size;
		fileSize -= size;
	}

	// Singly Indirect block ptr
	if (inode->block[INODE_SINGLY_INDIRECT_PTR_INDEX])
	{
		singlyBlock = (u32 *)kmalloc(disk->blockSize);
		if (singlyBlock == NULL)
		{
            KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
            status = STATUS_ALLOC_FAILED;
            goto clean;
		}

		unsigned long offset = inode->block[INODE_SINGLY_INDIRECT_PTR_INDEX] * disk->blockSize;
		int ret = AtaRead(disk->device, singlyBlock, offset, disk->blockSize);

		if (ret < 0)
		{
            KLOG(LOG_ERROR, "AtaRead() failed for singly indirect block ptr (returned %d)", ret);
            status = STATUS_FAILURE;
            goto clean;
		}

		for (int i = 0; i < disk->blockSize / sizeof(u32) && singlyBlock[i]; i++)
		{
			offset = singlyBlock[i] * disk->blockSize;
			int ret = AtaRead(disk->device, block, offset, disk->blockSize);

			if (ret < 0)
			{
                KLOG(LOG_ERROR, "AtaRead() failed for singly indirect block ptr (returned %d)", ret);
                status = STATUS_FAILURE;
                goto clean;
			}

			size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
			MmCopy((u8 *)block, (u8 *)(localFile + fileOffset), size);
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
                KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
                status = STATUS_ALLOC_FAILED;
                goto clean;
			}
		}

		doublyBlock = (u32 *)kmalloc(disk->blockSize);
		if (doublyBlock == NULL)
		{
            KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
            status = STATUS_ALLOC_FAILED;
            goto clean;
		}

		unsigned long offset = inode->block[INODE_DOUBLY_INDIRECT_PTR_INDEX] * disk->blockSize;
		int ret = AtaRead(disk->device, singlyBlock, offset, disk->blockSize);

		if (ret < 0)
		{
            KLOG(LOG_ERROR, "AtaRead() failed for singly indirect block ptr (returned %d)", ret);
            status = STATUS_FAILURE;
            goto clean;
		}

		for (int i = 0; i < disk->blockSize / sizeof(u32) && singlyBlock[i]; i++)
		{
			offset = singlyBlock[i] * disk->blockSize;
			ret = AtaRead(disk->device, doublyBlock, offset, disk->blockSize);

			if (ret < 0)
			{
                KLOG(LOG_ERROR, "AtaRead() failed for doubly indirect block ptr (returned %d)", ret);
                status = STATUS_FAILURE;
                goto clean;
			}

			for (int j = 0; j < disk->blockSize / sizeof(u32) && doublyBlock[j]; j++)
			{
				offset = doublyBlock[i] * disk->blockSize;
				int ret = AtaRead(disk->device, block, offset, disk->blockSize);

				if (ret < 0)
				{
                    KLOG(LOG_ERROR, "AtaRead() failed for doubly indirect block ptr (returned %d)", ret);
                    status = STATUS_FAILURE;
                    goto clean;
				}

				size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
				MmCopy((u8 *)block, (u8 *)(localFile + fileOffset), size);
				fileOffset += size;
				fileSize -= size;
			}
		}
	}

	// TODO : triply indirect, mais faut faire du ménage, trop l'bordel
	if (inode->block[INODE_TRIPLY_INDIRECT_PTR_INDEX])
	{
		KLOG(LOG_WARNING, "TRIPLY INDIRECT PTR NOT SUPPORTED ");
	}

    *file = localFile;
    localFile = NULL;

    status = STATUS_SUCCESS;

clean:
	if (localFile != NULL)
	{
		kfree(localFile);
        localFile = NULL;
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

	return status;
}

void Ext2FreeDisk(Ext2Disk * disk)
{
    if (disk == NULL)
    {
        KLOG(LOG_ERROR, "Invalid disk parameter");
        return;
    }

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