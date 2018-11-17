#include <kernel/drivers/proc_io.h>
#include <kernel/lib/types.h>

#include "serial.h"

static int ReceivedData (u16 port);
static int IsTransmitEmpty (u16 port);
static void SerialInitPort(u16 port);

void SerialInit ()
{
	SerialInitPort(COM1_PORT);
	SerialInitPort(COM2_PORT);
}

char SerialRead (u16 port)
{
    while (ReceivedData(port) == 0);

    return inb (port);
}

void SerialWrite (u16 port, char c)
{
    while (IsTransmitEmpty(port) == 0);

    outb (port, c);
}

static void SerialInitPort(u16 port)
{
	outb(port + 1, 0x00); // Desactive les interruptions
	outb(port + 3, 0x80); // Active le DLAB (Divisor Latch Access Bit)
	outb(port + 0, 0x03); // 38400 baud (115200 / 3), bits de poid faible
	outb(port + 1, 0x00); //                                       fort
	outb(port + 3, 0x03); // 8 bits, pas de parite, un bit de stop
	outb(port + 2, 0xC7);
	outb(port + 4, 0x0B);
	outb(port + 1, 0x01);
}

static int ReceivedData(u16 port)
{
    return inb (port + 5) & 1;
}

static int IsTransmitEmpty (u16 port)
{
    return inb (port + 5) & 0x20;
}
