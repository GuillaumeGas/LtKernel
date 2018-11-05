#pragma once

#include <kernel/drivers/ata.h>

// tmp, le temps d'avoir un truc générique
typedef AtaDevice Device;

struct LtDisk
{
	Device * device;
} typedef LtDisk;

struct LtFile
{
	long int size;
	void * content;
} typedef LtFile;

LtDisk * LtDiskCreate(Device * device);
LtFile * LtReadFile(LtDisk * disk, unsigned long offset);