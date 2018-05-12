#include "serial.h"
#include "proc_io.h"

static int received_data ();
static int is_transmit_empty ();

void init_serial ()
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

char read_serial ()
{
    while (received_data() == 0);

    return inb (COM1_PORT);
}

void write_serial (char c)
{
    while (is_transmit_empty () == 0);

    outb (COM1_PORT, c);
}

static int received_data ()
{
    return inb (COM1_PORT + 5) & 1;
}

static int is_transmit_empty ()
{
    return inb (COM1_PORT + 5) & 0x20;
}
