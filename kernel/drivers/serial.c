#include <kernel/drivers/proc_io.h>

#include "serial.h"

static int ReceivedData ();
static int IsTransmitEmpty ();

void SerialInit ()
{
    outb (COM1_PORT + 1, 0x00); // Desactive les interruptions
    outb (COM1_PORT + 3, 0x80); // Active le DLAB (Divisor Latch Access Bit)
    outb (COM1_PORT + 0, 0x03); // 38400 baud (115200 / 3), bits de poid faible
    outb (COM1_PORT + 1, 0x00); //                                       fort
    outb (COM1_PORT + 3, 0x03); // 8 bits, pas de parite, un bit de stop
    outb (COM1_PORT + 2, 0xC7);
    outb (COM1_PORT + 4, 0x0B);
    outb (COM1_PORT + 1, 0x01);
}

char SerialRead ()
{
    while (ReceivedData() == 0);

    return inb (COM1_PORT);
}

void SerialWrite (char c)
{
    while (IsTransmitEmpty() == 0);

    outb (COM1_PORT, c);
}

static int ReceivedData()
{
    return inb (COM1_PORT + 5) & 1;
}

static int IsTransmitEmpty ()
{
    return inb (COM1_PORT + 5) & 0x20;
}
