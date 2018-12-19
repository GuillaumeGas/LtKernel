#include "ltdbg.h"
#include "ltdbgcommands.h"
#include "ltdbgcom.h"

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

//static KeBreakpoint * BreakpointDefinedAt(u32 addr);

//static void WaitForDbgCommand(KeDebugContext * context);
//static void HandleBreakpoint(KeDebugContext * context, KeBreakpoint * bp);
//
//static void SendContext(KeDebugContext * context);
//static void SendResponseWithoutContext();
//static void SendResponseWithContext(KeDebugContext * context);
//static void SendResponseFailure();

static void WaitForConnectCommand(KeDebugContext * context);
static void WaitForPacket(KeDebugContext * context);

//static BOOL StepCommand(KeDebugContext * context);
//static BOOL ContinueCommand(KeDebugContext * context);
//static BOOL RegistersCommand(KeDebugContext * context);
//static BOOL DisassCommand(KeDebugContext * context);
//static BOOL StackTraceCommand(KeDebugContext * context);
//static BOOL MemoryCommand(KeDebugContext * context);
//static BOOL BreakpointCommand(KeDebugContext * context);

static BOOL gDbgInitialized = FALSE;
static CommandId gLastCommandId = CMD_UNKNOWN;
static List * gBpList = NULL;

void DbgInit()
{
	// Modifier les entrées 1 et 3 de l'idt (debug et breakpoint)
	IdtInitDescriptor((u32)_asm_debug_isr, CPU_GATE, 1);
	IdtInitDescriptor((u32)_asm_breakpoint_isr, CPU_GATE, 3);

	DISABLE_IRQ();
	IdtReload();
	ENABLE_IRQ();

	gBpList = ListCreate();
}

void DebugIsr(KeDebugContext * context)
{
	kprint("Debug interrupt !\n");
}

void BreakpointIsr(KeDebugContext * context)
{
	if (gDbgInitialized == FALSE)
	{
		WaitForConnectCommand(context);
	}
	else
	{
		WaitForPacket(context);
	}
}

static void WaitForConnectCommand(KeDebugContext * context)
{
	KeDebugRequest request = { 0 };
	KeDebugResponse response = { 0 };
	KeStatus status = STATUS_FAILURE;

	if (context == NULL)
	{
		//TODO : logger
		return;
	}

	u8 byte = ReadByte();
	if (byte != 1)
		kprint("Wrong startup byte !\n");
	else
		gDbgInitialized = TRUE;

	WriteByte(1);

	status = RecvRequest(&request);
	if (status != STATUS_SUCCESS)
	{
		// TODO : logger
		return;
	}

	if (request.command != CMD_CONNECT)
	{
		// TODO : logger
		return;
	}

	response.header.command = CMD_CONNECT;
	response.header.status = DBG_STATUS_SUCCESS;
	response.header.dataSize = 0;
	response.header.context = *context;
	response.data = NULL;

	SendResponse(&response);

	gDbgInitialized = TRUE;

	kprint("[DBG] LtDbg connected !\n");
}

static void WaitForPacket(KeDebugContext * context)
{
	KeDebugRequest request = { 0 };
	KeDebugResponse response = { 0 };
	KeStatus status = STATUS_FAILURE;

	if (context == NULL)
	{
		//TODO : logger
		return;
	}

	status = RecvRequest(&request);
	if (status != STATUS_SUCCESS)
	{
		// TODO : logger
		return;
	}

	kprint("Param size: %d\n", request.paramSize);

	switch (request.command)
	{
	case CMD_STEP:
		kprint("[DBG] Step command\n");
		break;
	case CMD_CONTINUE:
		kprint("[DBG] Continue command\n");
		break;
	case CMD_REGISTERS:
		kprint("[DBG] Registers command\n");
		break;
	case CMD_DISASS:
		kprint("[DBG] Disass command\n");
		break;
	case CMD_STACK_TRACE:
		kprint("[DBG] Stack trace command\n");
		break;
	case CMD_MEMORY:
		kprint("[DBG] Memory command\n");
		break;
	case CMD_BP:
		kprint("[DBG] Breakpoint command\n");
		break;
	default:
		kprint("[DBG] Undefined debug command\n");
	}

	response.header.command = request.command;
	response.header.context = *context;
	response.header.dataSize = 0;
	response.header.status = DBG_STATUS_SUCCESS;
	response.data = NULL;

	status = SendResponse(&response);
}
//
//static void WaitForDbgCommand(KeDebugContext * context)
//{
//	BOOL _continue = FALSE;
//
//	while (_continue == FALSE)
//	{
//		CommandId commandId = (CommandId)ReadByte();
//
//		switch (commandId)
//		{
//		case CMD_STEP:
//			_continue = StepCommand(context);
//			break;
//		case CMD_CONTINUE:
//			_continue = ContinueCommand(context);
//			break;
//		case CMD_REGISTERS:
//			_continue = RegistersCommand(context);
//			break;
//		case CMD_DISASS:
//			_continue = DisassCommand(context);
//			break;
//		case CMD_STACK_TRACE:
//			_continue = StackTraceCommand(context);
//			break;
//		case CMD_MEMORY:
//			_continue = MemoryCommand(context);
//			break;
//		case CMD_BP:
//			_continue = BreakpointCommand(context);
//			break;
//		default:
//			kprint("[DBG] Undefined debug command\n");
//		}
//
//		gLastCommandId = commandId;
//	}
//}
//
//static void SendContext(KeDebugContext * context)
//{
//	WriteBytes((u8 *)context, sizeof(KeDebugContext));
//}
//
//static void SendResponseWithoutContext()
//{
//	//WriteByte(KDBG_RESPONSE_WITHOUT_CONTEXT);
//}
//
//static void SendResponseWithContext(KeDebugContext * context)
//{
//	//WriteByte(KDBG_RESPONSE_WITH_CONTEXT);
//	SendContext(context);
//}
//
//static void SendResponseFailure()
//{
//	//WriteByte(KDBG_RESPONSE_FAILURE);
//}
//
//static KeBreakpoint * BreakpointDefinedAt(u32 addr)
//{
//	ListElem * elem = gBpList;
//	ListElem * next = NULL;
//
//	if (elem == NULL)
//		return NULL;
//
//	next = elem->next;
//
//	while (elem != NULL)
//	{
//		KeBreakpoint * bp = (KeBreakpoint *)elem->data;
//		if (bp->addr == addr)
//			return bp;
//
//		elem = next;
//
//		if (next != NULL)
//			next = next->next;
//	}
//
//	return NULL;
//}
//
//static void HandleBreakpoint(KeDebugContext * context, KeBreakpoint * bp)
//{
//	// TODO : trouver un moyen de restaurer le breakpoint après coup
//
//	context->eip--;
//
//	u8 * instAddr = (u8 *)context->eip;
//	*instAddr = bp->savedInstByte;
//}
//
//static BOOL StepCommand(KeDebugContext * context)
//{
//	kprint("StepCommand\n");
//	context->eflags |= TRAP_FLAG_MASK;
//	SendResponseWithoutContext();
//	return TRUE;
//}
//
//static BOOL ContinueCommand(KeDebugContext * context)
//{
//	kprint("ContinueCommand\n");
//	context->eflags &= 0x0FEFF;
//	SendResponseWithoutContext();
//	return TRUE;
//}
//
//static BOOL RegistersCommand(KeDebugContext * context)
//{
//	kprint("RegistersCommand\n");
//	SendResponseWithContext(context);
//	return FALSE;
//}
//
//static BOOL DisassCommand(KeDebugContext * context)
//{
//	unsigned int size = 0;
//
//	kprint("DisassCommand\n");
//
//	ReadBytes((u8*)&size, sizeof(unsigned int));
//
//	WriteBytes((u8*)&context->eip, sizeof(unsigned int));
//	WriteBytes((u8*)context->eip, size * DEFAULT_ASM_BUFFER_SIZE);
//
//	SendResponseWithoutContext();
//	return FALSE;
//}
//
//static BOOL StackTraceCommand(KeDebugContext * context)
//{
//	kprint("StackTraceCommand\n");
//
//	u8 * buffer = NULL;
//	List * list = ListCreate();
//	unsigned int nbPtr = 0;
//	u32 * ebp = (u32*)context->ebp;
//	unsigned int bufferSize = 0;
//
//	ListPush(list, (void *)context->eip);
//	nbPtr++;
//
//	while (ebp != NULL)
//	{
//		ListPush(list, (void *)ebp[1]);
//		ebp = (u32 *)ebp[0];
//		nbPtr++;
//	}
//
//	bufferSize = nbPtr * sizeof(u32);
//	
//	buffer = (u8 *)kmalloc(bufferSize);
//	if (buffer == NULL)
//	{
//		kprint("[DBG ERROR] StackTraceCommand() : Can't allocate %d bytes\n", bufferSize);
//		goto clean;
//	}
//
//	unsigned int index = 0;
//	unsigned int * addresses = (unsigned int *)buffer;
//	while (list != NULL)
//	{
//		addresses[index++] = (unsigned int)ListPop(&list);
//	}
//
//	WriteBytes((u8 *)&bufferSize, sizeof(unsigned int));
//	WriteBytes((u8 *)buffer, bufferSize);
//
//clean:
//	ListDestroy(list);
//	SendResponseWithoutContext();
//	return FALSE;
//}
//
//static BOOL MemoryCommand(KeDebugContext * context)
//{
//	unsigned int addr = 0;
//	unsigned int size = 0;
//	PageDirectoryEntry * currentPd = NULL;
//
//	ReadBytes((u8 *)&addr, sizeof(unsigned int));
//	ReadBytes((u8 *)&size, sizeof(unsigned int));
//
//	// Changer de répertoire de table de page (à récupérer dans cr3 ?)
//	// Garder en mémoire le courant
//	currentPd = _getCurrentPagesDirectory();
//	_setCurrentPagesDirectory((PageDirectoryEntry *)context->cr3);
//
//	// Vérifier si l'adresse demandée est accessible et renvoyer le résultat 1 ou 0
//	if (!IsVirtualAddressAvailable(addr))
//	{
//		WriteByte(FALSE);
//		goto clean;
//	}
//	
//	// Vérifier si l'adresse de fin est également accessible
//	if (!IsVirtualAddressAvailable(addr + size - 1))
//	{
//		WriteByte(FALSE);
//		goto clean;
//	}
//	
//	WriteByte(TRUE);
//
//	// Envoyer les données
//	WriteBytes((u8 *)addr, size);
//
//clean:
//    // Restaurer répertoire de pages
//    _setCurrentPagesDirectory(currentPd);
//	SendResponseWithoutContext();
//	return FALSE;
//}
//
//static BOOL BreakpointCommand(KeDebugContext * context)
//{
//	const u8 INT_3_INST = 0xCC;
//
//	u8 * addr = NULL;
//	static int breakpointId = 0;
//	KeBreakpoint * bp = NULL;
//
//	ReadBytes((u8 *)&addr, sizeof(u32));
//
//	if (!IsVirtualAddressAvailable((u32)addr))
//	{
//		WriteByte(FALSE);
//		goto clean;
//	}
//
//	bp = (KeBreakpoint *)kmalloc(sizeof(KeBreakpoint));
//	if (bp == NULL)
//	{
//		kprint("[DBG ERROR] BreakpointCommand() : Can't allocate %d bytes\n", sizeof(KeBreakpoint));
//		WriteByte(FALSE);
//		goto clean;
//	}
//
//	bp->addr = (u32)addr;
//	bp->id = breakpointId++;
//	bp->savedInstByte = *addr;
//	bp->state = BP_ENABLED;
//
//	ListPush(gBpList, bp);
//
//	// TODO : demander au debugger la taille de l'instruction
//	*addr = INT_3_INST;
//
//	WriteByte(TRUE);
//
//clean:
//	SendResponseWithoutContext();
//	return FALSE;
//}