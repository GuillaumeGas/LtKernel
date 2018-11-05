#include "ltfs.h"

#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/kmalloc.h>

LtDisk * LtDiskCreate(Device * device)
{
	LtDisk * disk = NULL;

	disk = (LtDisk *)kmalloc(sizeof(LtDisk));
	if (disk == NULL)
	{
		kprint("LtDiskCreate() failed, couldn't allocate memory\n");
		return NULL;
	}
	return disk;
}

LtFile * LtReadFile(LtDisk * disk, unsigned long offset)
{
	LtFile * file = NULL;
	long int fileSize = 0;
	unsigned long ltFileSize = 0;
	int ret = 0;

	ret = AtaRead(disk->device, &fileSize, 0, sizeof(long int));
	if (ret <= 0)
	{
		kprint("LtReadFile() : AtaRead() failed with code : %d\n", ret);
		return NULL;
	}

	if (fileSize <= 0)
	{
		kprint("LtReadFile() : fileSize <= 0 (%d)!\n", fileSize);
		return NULL;
	}

	ltFileSize = fileSize + sizeof(long int);
	file = (LtFile *)kmalloc(fileSize + sizeof(long int));
	if (file == NULL)
	{
		kprint("LtReadFile() : can't allocate memory for LtFile\n");
		return NULL;
	}

	ret = AtaRead(disk->device, file, 0, ltFileSize);
	if (ret <= 0)
	{
		kprint("LtReadFile() : AtaRead() failed with code %d\n", ret);
		goto clean;
	}

	return file;

clean:
	if (file != NULL)
	{
		kfree(file);
		file = NULL;
	}

	return NULL;
}