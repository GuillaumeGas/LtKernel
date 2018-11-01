#include <kernel/lib/stdlib.h>
#include <kernel/drivers/proc_io.h>
#include <kernel/lib/stdio.h>	

#include "ata.h"

#define ATA_PRIMARY_IRQ 14
#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_DCR_AS 0x3F6

#define ATA_SECONDARY_IRQ 15
#define ATA_SECONDARY_IO 0x170
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

#define ATA_MASTER 0x00
#define ATA_SLAVE  0x01

#define MODULE_NAME ata

#define EIO 5
#define ENOSYS 35
#define ENOMEM 12

static struct device *ata_devices[4];
static int n_ata_devices;

struct dma_prdt {
	u32 prdt_offset;
	u16 prdt_bytes;
	u16 prdt_last;
} __packed;

struct ata_dma_priv {
	int adp_busmaster;
	int adp_last_count;
	char *adp_dma_area;
	struct dma_prdt *adp_dma_prdt;
};

void ide_select_drive(u8 bus, u8 i)
{
	if (bus == ATA_PRIMARY)
	{
		if (i == ATA_MASTER)
		{
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		}
		else
		{
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
		}
	}
	else
	{
		if (i == ATA_MASTER)
		{
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		}
		else
		{
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
		}
	}
}

void ide_400ns_delay(u16 io)
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

void
ide_poll(u16 io)
{
	u8 status;

	/* read the ALTSTATUS 4 times */
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);

	/* now wait for the BSY bit to clear */
	while ((status = inb(io + ATA_REG_STATUS)) & ATA_SR_BSY)
		;

	status = inb(io + ATA_REG_STATUS);

	if (status & ATA_SR_ERR)
		panic("ATA ERR bit is set\n");

	return;
}

int
ata_status_wait(int io_base, int timeout) {
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

void
ata_io_wait(int io_base) {
	inb(io_base + ATA_REG_ALTSTATUS);
	inb(io_base + ATA_REG_ALTSTATUS);
	inb(io_base + ATA_REG_ALTSTATUS);
	inb(io_base + ATA_REG_ALTSTATUS);
}

int
ata_wait(int io, int adv)
{
	u8 status = 0;

	ata_io_wait(io);

	status = ata_status_wait(io, -1);

	if (adv) {
		status = inb(io + ATA_REG_STATUS);
		if (status & ATA_SR_ERR) return 1;
		if (status & ATA_SR_DF)  return 1;
		if (!(status & ATA_SR_DRQ)) return 1;
	}

	return 0;
}

int
ata_read_one_sector_pio(char *buf, int lba)
{
	u16 io = ATA_SECONDARY_IO;
	u8  dr = ATA_MASTER;

	u8 cmd = 0xE0;
	int errors = 0;
	u8 slavebit = 0x00;

	//kprint("ata: lba: %d\n", lba);
try_a:
	outb(io + ATA_REG_CONTROL, 0x02);

	ata_wait(io, 0);

	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0F))));
	outb(io + ATA_REG_FEATURES, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (u8)(lba));
	outb(io + ATA_REG_LBA1, (u8)(lba >> 8));
	outb(io + ATA_REG_LBA2, (u8)(lba >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	if (ata_wait(io, 1)) {
		errors++;
		if (errors > 4)
			return -EIO;

		goto try_a;
	}

	for (int i = 0; i < 256; i++) {
		u16 d = inw(io + ATA_REG_DATA);
		*(u16 *)(buf + i * 2) = d;
	}

	ata_wait(io, 0);
	return 0;
}

int
ata_read_pio(struct device *dev, void *buf, int count)
{
	unsigned long pos = dev->pos;
	int rc = 0, read = 0;

	DISABLE_IRQ();

	for (int i = 0; i < count; i++)
	{
		rc = ata_read_one_sector_pio(buf, pos + i);
		if (rc == -EIO)
			return -EIO;
		buf += 512;
		read += 512;
	}
	dev->pos += count;

	ENABLE_IRQ();
	return count;
}

int
ata_write_one_sector_pio(u16 *buf, int lba)
{
	u16 io = ATA_SECONDARY_IO;
	u8  dr = ATA_MASTER;

	u8 cmd = 0xE0;
	u8 slavebit = 0x00;

	outb(io + ATA_REG_CONTROL, 0x02);

	ata_wait(io, 0);

	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0F))));
	ata_wait(io, 0);
	outb(io + ATA_REG_FEATURES, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (u8)(lba));
	outb(io + ATA_REG_LBA1, (u8)(lba >> 8));
	outb(io + ATA_REG_LBA2, (u8)(lba >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
	ata_wait(io, 0);

	for (int i = 0; i < 256; i++) {
		outw(io + ATA_REG_DATA, buf[i]);
		asm volatile("nop; nop; nop");
	}
	outb(io + 0x07, ATA_CMD_CACHE_FLUSH);

	ata_wait(io, 0);

	return 0;
}

int
ata_write_pio(struct device *dev, void *buf, int count)
{
	unsigned long pos = dev->pos;

	DISABLE_IRQ();
	for (int i = 0; i < count; i++)
	{
		ata_write_one_sector_pio(buf, pos + i);
		buf += 512;
		for (int j = 0; j < 1000; j++)
			;
	}
	dev->pos += count;
	ENABLE_IRQ();
	return count;
}

int
ata_sync()
{
	return -ENOSYS;
}

int
ide_identify(void)
{
	u16 io = 0;
	/* XXX: support multiple ATA devices */
	io = 0x170;
	outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	u8 status = inb(io + ATA_REG_STATUS);
	if (status)
	{
		/* read the IDENTIFY data */
		struct device *dev;
		void *ide_buf = kmalloc(512);
		if (!ide_buf)
			return -ENOMEM;

		dev = kmalloc(sizeof(*dev));
		if (!dev) {
			kfree(ide_buf);
			return -ENOMEM;
		}

		for (int i = 0; i < 256; i++)
			*(u16 *)(ide_buf + i * 2) = inw(io + ATA_REG_DATA);

		kfree(ide_buf);

		dev->pos = 0;
		dev->type = DEV_TYPE_BLOCK;
		dev->subtype = DEV_TYPE_BLOCK_ATA;
		dev->priv = NULL;
		dev->name = "ata";
		//device_register(dev);
		ata_devices[n_ata_devices++] = dev;
		return 1;
	}
	else {
		kprint("ata: IDENTIFY error on b0d0 -> no status\n");
		return 0;
	}
}


void
ata_probe(void)
{
	int devs = 0;
	if (ide_identify() > 0) {
		kprint("ata: primary master is online\n");
		devs++;
	}
	kprint("ata: %d devices brought online\n", devs);
}

void
ide_prim_irq(struct pt_regs *r)
{
	return;
}

void
ata_init(void)
{
	kprint("ata: using PIO mode, disregarding secondary\n");
	//intr_register_hw(0x20 + ATA_PRIMARY_IRQ, ide_prim_irq);
	ata_probe();

	struct device * device = ata_devices[0];
	char * buf = (char*)kmalloc(512);
	StrCpy("Hello world !\n", buf);
	ata_write_pio(device, buf, 1);
}
