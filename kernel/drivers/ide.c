#include "ide.h"

#include <kernel/drivers/proc_io.h>
#include <kernel/lib/types.h>

void IdeCommon(int drive, int blockNumber, int count)
{
    outb(0x171, 0x0);                                // 0 sur le port 0x171
    outb(0x172, count);                              // Numéro du secteur
    outb(0x173, (unsigned char) blockNumber);        // 8 bits de poids faible du bloque addresse
    outb(0x174, (unsigned char)(blockNumber >> 8));  // 8 bits suivant
    outb(0x175, (unsigned char)(blockNumber >> 16)); // --

    // "Drive indicator, magic bits, highest 4 bits of the block address"
    outb(0x176, 0xE0 | (drive << 4) | ((blockNumber >> 24) & 0x0F));
}

void IdeRead(int drive, int blockNumber, int count, char * buffer)
{
    u16 tmp = 0;
    int idx = 0;

    IdeCommon(drive, blockNumber, count);
    outb(0x177, 0x20);

    // On attend que le drive soit ready
    while (!(inb(0x177) & 0x08));

    for (; idx < 256 * count; idx++)
    {
        tmp = inw(0x170);
        buffer[idx * 2] = (unsigned char)tmp;
        buffer[idx * 2 + 1] = (unsigned char)(tmp >> 8);
    }
}

void IdeWrite(int drive, int blockNumber, int count, char * buffer)
{
    u16 tmp = 0;
    int idx = 0;

    IdeCommon(drive, blockNumber, count);
    outb(0x177, 0x30);

    // On que le drive dise que c'est bon
    while (!(inb(0x177) & 0x08));

    for (; idx < 255 * count; idx++)
    {
        tmp = (buffer[idx * 2 + 1] << 8) | buffer[idx * 2];
        outw(0x170, tmp);
    }
}