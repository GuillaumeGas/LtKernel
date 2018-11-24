#include "ltdbg.h"

#include <kernel/init/idt.h> 
#include <kernel/init/isr.h>

#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/serial.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>

#define TRAP_FLAG_MASK 0x100;

void _asm_debug_isr();
void _asm_breakpoint_isr();

static u8 ReadByte();
static void WriteByte(u8 byte);
static void WriteBytes(u8 * buffer, unsigned int bufferSize);

static void WaitForDbgCommand(KeDebugContext * context);
static BOOL StepCommand(KeDebugContext * context);
static BOOL ContinueCommand(KeDebugContext * context);
static BOOL RegistersCommand(KeDebugContext * context);
static BOOL DisassCommand(KeDebugContext * context);
static BOOL StackTraceCommand(KeDebugContext * context);

void DbgInit()
{
	// Modifier les entrées 1 et 3 de l'idt (debug et breakpoint)
	IdtInitDescriptor((u32)_asm_debug_isr, CPU_GATE, 1);
	IdtInitDescriptor((u32)_asm_breakpoint_isr, CPU_GATE, 3);

	DISABLE_IRQ();
	IdtReload();
	ENABLE_IRQ();
}

void DebugIsr(KeDebugContext * context)
{
	kprint("Debug interrupt !\n");
	WaitForDbgCommand(context);
}

void BreakpointIsr(KeDebugContext * context)
{
	u8 byte = ReadByte();
	if (byte != 1)
		kprint("Wrong startup byte !\n");

	WriteByte(1);
	kprint("[DBG] LtDbg connected\n");
	
	WaitForDbgCommand(context);
}

static u8 ReadByte()
{
	return SerialRead(COM2_PORT);
}

static void WriteByte(u8 byte)
{
	SerialWrite(COM2_PORT, byte);
}

static void WriteBytes(u8 * buffer, unsigned int bufferSize)
{
	for (unsigned int i = 0; i < bufferSize; i++)
	{
		WriteByte(buffer[i]);
	}
}

static void WaitForDbgCommand(KeDebugContext * context)
{
	char cmd = 0;
	BOOL _continue = FALSE;

	while (_continue == FALSE)
	{
		cmd = ReadByte();

		switch (cmd)
		{
		case 'p':
			_continue = StepCommand(context);
			break;
		case 'c':
			_continue = ContinueCommand(context);
			break;
		case 'r':
			_continue = RegistersCommand(context);
			break;
		case 'd':
			_continue = DisassCommand(context);
			break;
		case 's':
			_continue = StackTraceCommand(context);
			break;
		default:
			kprint("[DBG] Undefined debug command\n");
		}
	}
}

static BOOL StepCommand(KeDebugContext * context)
{
	kprint("StepCommand\n");
	context->eflags |= TRAP_FLAG_MASK;
	return TRUE;
}

static BOOL ContinueCommand(KeDebugContext * context)
{
	kprint("ContinueCommand\n");
	context->eflags &= 0x0FEFF;
	return TRUE;
}

static BOOL RegistersCommand(KeDebugContext * context)
{
	kprint("RegistersCommand\n");
	WriteBytes((u8*)context, sizeof(KeDebugContext));
	return FALSE;
}

static BOOL DisassCommand(KeDebugContext * context)
{
	kprint("DisassCommand\n");
	WriteBytes((u8*)&context->eip, sizeof(unsigned int));
	WriteBytes((u8*)context->eip, 20);
	return FALSE;
}

static BOOL StackTraceCommand(KeDebugContext * context)
{
	kprint("StackTraceCommand\n");
	u32 addrs[2];
	u32 * ebp = (u32*)context->ebp;
	addrs[0] = context->eip;
	addrs[1] = ebp[1];
	WriteBytes((u8*)addrs, 2 * sizeof(u32));
	return FALSE;
}