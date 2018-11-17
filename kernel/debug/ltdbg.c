#include "ltdbg.h"

#include <kernel/init/idt.h> 

#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/serial.h>

#include <kernel/lib/stdio.h>

void _asm_debug_isr();
void _asm_breakpoint_isr();

void DbgInit()
{
	// Modifier les entrées 1 et 3 de l'idt (debug et breakpoint)
	IdtInitDescriptor((u32)_asm_debug_isr, CPU_GATE, 1);
	IdtInitDescriptor((u32)_asm_breakpoint_isr, CPU_GATE, 3);

	DISABLE_IRQ();
	IdtReload();
	ENABLE_IRQ();
}

void DebugIsr()
{

}

void BreakpointIsr()
{
	char test = 0;
	test = SerialRead(COM2_PORT);

	kprint("Bp ! (test : %d)\n", test);

	kprint("Sending reply...\n");
	SerialWrite(COM2_PORT, (char)1);
	
	kprint("Pause\n");

	while (1);
}