#include "disk.h"

#include <kernel/drivers/proc_io.h>
#include <kernel/lib/stdio.h>

// Base : https://github.com/AlgorithMan-de/wyoos/commit/fa076f542ab5f16a32bd141e4b44c62ab0bac9b8

#define ATA_MASTER_VALUE     0xA0
#define ATA_SLAVE_VALUE      0xB0
#define ATA_IDENTIFY_COMMAND 0xEC

AtaInfo AtaCreate(AtaType type, u16 portBase)
{
	AtaInfo info = { 0 };

	info.type = type == ATA_MASTER ? ATA_MASTER_VALUE : ATA_SLAVE_VALUE;
	info.dataPort = portBase;
	info.errorPort = portBase + 0x1;
	info.sectorCountPort = portBase + 0x2;
	info.lbaLowPort = portBase + 0x3;
	info.lbaMidPort = portBase + 0x4;
	info.lbaHiPort = portBase + 0x5;
	info.devicePort = portBase + 0x6;
	info.commandPort = portBase + 0x7;
	info.controlPort = portBase + 0x206;

	return info;
}

void AtaIdentify(AtaInfo * info)
{
	u8 status = 0;

	outb(info->devicePort, info->type);
	outb(info->controlPort, 0);

	//outb(info->devicePort, 0xA0);
	//status = inb(info->commandPort);
	//if (status == 0xFF)
	//{
	//	kprint("[ATA][INFO] : AtaIdentify() retuned for 0xA0 command, status == 0xFF\n");
	//	return;
	//}

	outb(info->devicePort, info->type);
	outb(info->sectorCountPort, 0);
	outb(info->lbaLowPort, 0);
	outb(info->lbaMidPort, 0);
	outb(info->lbaHiPort, 0);
	outb(info->commandPort, ATA_IDENTIFY_COMMAND);

	status = inb(info->commandPort);
	if (status == 0x00)
	{
		kprint("[ATA][INFO] : AtaIdentify() retuned for identify command, status == 0xFF\n");
		return;
	}

	while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
	{
		status = inb(info->commandPort);
	}

	if (status & 0x01)
	{
		kprint("[ATA][ERROR] : Identify() failed ! status & 0x01 == 1\n");
		return;
	}

	{
		int i = 0;
		for (; i < 256; i++)
		{
			u16 data = inw(info->dataPort);
			char * text = "  \0";
			text[0] = (data >> 8) & 0xFF;
			text[1] = data & 0xFF;
			kprint(text);
		}
		kprint("\n");
	}
}

void AtaRead(AtaInfo * info, u32 sectorNum, int count, u8 * buffer) {}
void AtaWrite(AtaInfo * info, u32 sectorNum, int count, u8 * buffer) {}

//void IdeCommon(int drive, int blockNumber, int count)
//{
//    outb(0x171, 0x0);                                // 0 sur le port 0x171
//    outb(0x172, count);                              // Numéro du secteur
//    outb(0x173, (unsigned char) blockNumber);        // 8 bits de poids faible du bloque addresse
//    outb(0x174, (unsigned char)(blockNumber >> 8));  // 8 bits suivant
//    outb(0x175, (unsigned char)(blockNumber >> 16)); // --
//
//    // "Drive indicator, magic bits, highest 4 bits of the block address"
//    outb(0x176, 0xE0 | (drive << 4) | ((blockNumber >> 24) & 0x0F));
//}
//
//void IdeRead(int drive, int blockNumber, int count, char * buffer)
//{
//    u16 tmp = 0;
//    int idx = 0;
//
//    IdeCommon(drive, blockNumber, count);
//    outb(0x177, 0x20);
//
//    // On attend que le drive soit ready
//    while (!(inb(0x177) & 0x08));
//
//    for (; idx < 256 * count; idx++)
//    {
//        tmp = inw(0x170);
//        buffer[idx * 2] = (unsigned char)tmp;
//        buffer[idx * 2 + 1] = (unsigned char)(tmp >> 8);
//    }
//}
//
//void IdeWrite(int drive, int blockNumber, int count, char * buffer)
//{
//    u16 tmp = 0;
//    int idx = 0;
//
//    IdeCommon(drive, blockNumber, count);
//    outb(0x177, 0x30);
//
//    // On que le drive dise que c'est bon
//    while (!(inb(0x177) & 0x08));
//
//    for (; idx < 255 * count; idx++)
//    {
//        tmp = (buffer[idx * 2 + 1] << 8) | buffer[idx * 2];
//        outw(0x170, tmp);
//    }
//}