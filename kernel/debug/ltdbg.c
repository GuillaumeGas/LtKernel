#include "ltdbg.h"
#include "ltdbgcommands.h"

#include <kernel/init/idt.h> 
#include <kernel/init/isr.h>
#include <kernel/init/vmm.h>

#include <kernel/drivers/proc_io.h>
#include <kernel/drivers/serial.h>

#include <kernel/lib/stdio.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/list.h>
#include <kernel/lib/kmalloc.h>

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
static BOOL MemoryCommand(KeDebugContext * context);

static BOOL gDbgInitialized = FALSE;
static CommandId gLastCommandId = CMD_UNKNOWN;

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

	DisassCommand(context);

	WaitForDbgCommand(context);
}

void BreakpointIsr(KeDebugContext * context)
{
	if (gDbgInitialized == FALSE)
	{
		u8 byte = ReadByte();
		if (byte != 1)
			kprint("Wrong startup byte !\n");
		else
			gDbgInitialized = TRUE;

		WriteByte(1);
		kprint("[DBG] LtDbg connected\n");
	}

	WaitForDbgCommand(context);
}

static u8 ReadByte()
{
	return SerialRead(COM2_PORT);
}

static void ReadBytes(u8 * buffer, unsigned int size)
{
	if (buffer == NULL)
		return;

	for (int i = 0; i < size; i++)
		buffer[i] = ReadByte();
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
	BOOL _continue = FALSE;

	while (_continue == FALSE)
	{
		CommandId commandId = (CommandId)ReadByte();

		switch (commandId)
		{
		case CMD_STEP:
			_continue = StepCommand(context);
			break;
		case CMD_CONTINUE:
			_continue = ContinueCommand(context);
			break;
		case CMD_REGISTERS:
			_continue = RegistersCommand(context);
			break;
		case CMD_DISASS:
			_continue = DisassCommand(context);
			break;
		case CMD_STACK_TRACE:
			_continue = StackTraceCommand(context);
			break;
		case CMD_MEMORY:
			_continue = MemoryCommand(context);
			break;
		default:
			kprint("[DBG] Undefined debug command\n");
		}

		gLastCommandId = commandId;
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
	unsigned int size = 0;

	kprint("DisassCommand\n");

	ReadBytes((u8*)&size, sizeof(unsigned int));

	WriteBytes((u8*)&context->eip, sizeof(unsigned int));
	WriteBytes((u8*)context->eip, size * DEFAULT_ASM_BUFFER_SIZE);
	return FALSE;
}

static BOOL StackTraceCommand(KeDebugContext * context)
{
	kprint("StackTraceCommand\n");

	u8 * buffer = NULL;
	List * list = ListCreate();
	unsigned int nbPtr = 0;
	u32 * ebp = (u32*)context->ebp;
	unsigned int bufferSize = 0;

	ListPush(list, (void *)context->eip);
	nbPtr++;

	while (ebp != NULL)
	{
		ListPush(list, (void *)ebp[1]);
		ebp = (u32 *)ebp[0];
		nbPtr++;
	}

	bufferSize = nbPtr * sizeof(u32);
	
	buffer = (u8 *)kmalloc(bufferSize);
	if (buffer == NULL)
	{
		kprint("[DBG ERROR] StackTraceCommand() : Can't allocate %d bytes\n", bufferSize);
		goto clean;
	}

	unsigned int index = 0;
	unsigned int * addresses = (unsigned int *)buffer;
	while (list != NULL)
	{
		addresses[index++] = (unsigned int)ListPop(&list);
	}

	WriteBytes((u8 *)&bufferSize, sizeof(unsigned int));
	WriteBytes((u8 *)buffer, bufferSize);

clean:
	ListDestroy(list);

	return FALSE;
}

static BOOL MemoryCommand(KeDebugContext * context)
{
	unsigned int addr = 0;
	unsigned int size = 0;
	PageDirectoryEntry * currentPd = NULL;

	ReadBytes((u8 *)&addr, sizeof(unsigned int));
	ReadBytes((u8 *)&size, sizeof(unsigned int));

	// Changer de répertoire de table de page (à récupérer dans cr3 ?)
	// Garder en mémoire le courant
	currentPd = _getCurrentPagesDirectory();
	_setCurrentPagesDirectory((PageDirectoryEntry *)context->cr3);

	// Vérifier si l'adresse demandée est accessible et renvoyer le résultat 1 ou 0
	if (!IsVirtualAddressAvailable(addr))
	{
		WriteByte(FALSE);
		goto clean;
	}
	
	// Vérifier si l'adresse de fin est également accessible
	if (!IsVirtualAddressAvailable(addr + size - 1))
	{
		WriteByte(FALSE);
		goto clean;
	}
	
	WriteByte(TRUE);

	// Envoyer les données
	WriteBytes((u8 *)addr, size);

clean:
    // Restaurer répertoire de pages
    _setCurrentPagesDirectory(currentPd);

	return FALSE;
}