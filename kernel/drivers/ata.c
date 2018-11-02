#include <kernel/lib/stdlib.h>
#include <kernel/drivers/proc_io.h>
#include <kernel/lib/stdio.h>	
#include <kernel/lib/kmalloc.h>

#include "ata.h"

// Un exemple qui a l'air pas mal : https://github.com/CHOSTeam/CHicago/blob/master/kernel/arch/x86/io/ide.c

#define EIO 5
#define ENOSYS 35
#define ENOMEM 12

static int AtaStatusWait(int io_base, int timeout) 
{
	int status;

	if (timeout > 0) {
		int i = 0;
		while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
	}
	else {
		while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY);
	}
	return status;
}

static void AtaIoWait(int io_base) 
{
	inb(io_base + ATA_REG_ALTSTATUS);
	inb(io_base + ATA_REG_ALTSTATUS);
	inb(io_base + ATA_REG_ALTSTATUS);
	inb(io_base + ATA_REG_ALTSTATUS);
}

static int AtaWait(int io, int adv)
{
	u8 status = 0;

	AtaIoWait(io);

	status = AtaStatusWait(io, -1);

	if (adv) 
	{
		status = inb(io + ATA_REG_STATUS);
		if (status & ATA_SR_ERR) return 1;
		if (status & ATA_SR_DF)  return 1;
		if (!(status & ATA_SR_DRQ)) return 1;
	}

	return 0;
}

static int AtaReadSectorPio(AtaDevice * device, char * buf, int lba)
{
	u16 io = device->dataPort;

	u8 cmd = 0xE0;
	int errors = 0;

try_label:
	outb(io + ATA_REG_CONTROL, 0x02);

	AtaWait(io, 0);

	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0F))));
	outb(io + ATA_REG_FEATURES, 0x00);
	outb(device->sectorCountPort, 1);
	outb(device->lbaLowPort, (u8)(lba));
	outb(device->lbaMidPort, (u8)(lba >> 8));
	outb(device->lbaHiPort, (u8)(lba >> 16));
	outb(device->commandPort, ATA_CMD_READ_PIO);

	if (AtaWait(io, 1)) 
	{
		errors++;
		if (errors > 4)
			return -EIO;

		goto try_label;
	}

	for (int i = 0; i < 256; i++) 
	{
		u16 d = inw(device->dataPort);
		*(u16 *)(buf + i * 2) = d;
	}

	AtaWait(io, 0);
	return 0;
}

int AtaReadPio(AtaDevice * dev, void * buf, int count)
{
	unsigned long pos = dev->pos;
	int rc = 0, read = 0;

	DISABLE_IRQ();

	for (int i = 0; i < count; i++)
	{
		rc = AtaReadSectorPio(dev, buf, pos + i);
		if (rc == -EIO)
			return -EIO;
		buf += 512;
		read += 512;
	}
	dev->pos += count;

	ENABLE_IRQ();
	return count;
}

static int AtaWriteSectorPio(AtaDevice * device, u16 * buf, int lba)
{
	u16 io = device->dataPort;
	u8 cmd = 0xE0;

	outb(io + ATA_REG_CONTROL, 0x02);

	AtaWait(io, 0);

	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0F))));
	AtaWait(io, 0);
	outb(io + ATA_REG_FEATURES, 0x00);
	outb(device->sectorCountPort, 1);
	outb(device->lbaLowPort, (u8)(lba));
	outb(device->lbaMidPort, (u8)(lba >> 8));
	outb(device->lbaHiPort, (u8)(lba >> 16));
	outb(device->commandPort, ATA_CMD_WRITE_PIO);
	AtaWait(io, 0);

	for (int i = 0; i < 256; i++) 
	{
		outw(device->dataPort, buf[i]);
		asm volatile("nop; nop; nop");
	}
	outb(device->commandPort, ATA_CMD_CACHE_FLUSH);

	AtaWait(io, 0);

	return 0;
}

int AtaWritePio(AtaDevice * device, void * buf, int count)
{
	unsigned long pos = device->pos;

	DISABLE_IRQ();
	for (int i = 0; i < count; i++)
	{
		AtaWriteSectorPio(device, buf, pos + i);
		buf += 512;
		for (int j = 0; j < 1000; j++);
	}
	device->pos += count;
	ENABLE_IRQ();
	return count;
}

static int AtaIdentify(AtaDevice * device)
{
	outb(device->devicePort, device->type);
	outb(device->sectorCountPort, 0);
	outb(device->lbaLowPort, 0);
	outb(device->lbaMidPort, 0);
	outb(device->lbaHiPort, 0);
	outb(device->commandPort, ATA_CMD_IDENTIFY);

	u8 status = inb(device->dataPort + ATA_REG_STATUS);
	if (status)
	{
        for (int i = 0; i < 256; i++)
            inw(device->dataPort);

		return 1;
	}
	else 
	{
		kprint("ata: IDENTIFY error on b0d0 -> no status\n");
		return 0;
	}
}

AtaDevice AtaCreate(AtaIoPort ioPort, AtaType type)
{
	AtaDevice device = { 0 };
	
	device.type = type;
	device.dataPort = ioPort;
	device.errorPort = ioPort + 0x1;
	device.sectorCountPort = ioPort + 0x2;
	device.lbaLowPort = ioPort + 0x3;
	device.lbaMidPort = ioPort + 0x4;
	device.lbaHiPort = ioPort + 0x5;
	device.devicePort = ioPort + 0x6;
	device.commandPort = ioPort + 0x7;
	device.controlPort = ioPort + 0x206;
	device.pos = 0;
	
	return device;
}

int AtaInit(AtaDevice * ataDevice)
{
	kprint("Ata %s %s using PIO mode\n", ataDevice->dataPort == ATA_PRIMARY ? "Primary" : "Secondary", ataDevice->type == ATA_MASTER ? "Master" : "Slave");
	return AtaIdentify(ataDevice);
}
