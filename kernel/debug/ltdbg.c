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

#include <kernel/logger.h>

#define TRAP_FLAG_MASK 0x100;

// #define DEBUG_DEBUGGER

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

void _asm_debug_isr();
void _asm_breakpoint_isr();

//static KeBreakpoint * BreakpointDefinedAt(u32 addr);

static void WaitForConnectCommand(KeDebugContext * context);
static void WaitForPacket(KeDebugContext * context);

static void CleanupKeDebugResponse(KeDebugResponse * response);
static void CleanupKeDebugRequest(KeDebugRequest * request);

static BOOL ContinueCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
static BOOL QuitCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
static BOOL StepCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
static BOOL RegistersCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
static BOOL DisassCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
static BOOL StackTraceCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
static BOOL MemoryCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
//static BOOL BreakpointCommand(KeDebugContext * context);

static BOOL gDbgInitialized = FALSE;
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

static void BreakpointHit(KeDebugContext * context)
{
	KeDebugResponse response = { 0 };
	response.header.command = CMD_UNKNOWN;
	response.header.context = *context;
	response.header.dataSize = 0;
	response.header.status = DBG_STATUS_BREAKPOINT_REACHED;
	response.data = NULL;

	SendResponse(&response);
}

void DebugIsr(KeDebugContext * context)
{
	DKLOG(LOG_DEBUG, "Debug interrupt !");

	if (gDbgInitialized == FALSE)
	{
		KLOG(LOG_ERROR, "Debugger uninitialized !");
	}

	BreakpointHit(context);
	WaitForPacket(context);
}

void BreakpointIsr(KeDebugContext * context)
{
	DKLOG(LOG_DEBUG, "Debug breakpoint !");

	if (gDbgInitialized == FALSE)
	{
		WaitForConnectCommand(context);
	}
	else
	{
		BreakpointHit(context);
	}

	WaitForPacket(context);
}

static void WaitForConnectCommand(KeDebugContext * context)
{
	KeDebugRequest request = { 0 };
	KeDebugResponse response = { 0 };
	KeStatus status = STATUS_FAILURE;

	if (context == NULL)
	{
		KLOG(LOG_ERROR, "Invalid context parameter");
		return;
	}

	u8 byte = ReadByte();
	if (byte != 1)
		KLOG(LOG_ERROR, "Wrong startup byte !");
	else
		gDbgInitialized = TRUE;

	WriteByte(1);

	KLOG(LOG_INFO, "LtDbg trying to connect... waiting for a connect request...");

	status = RecvRequest(&request);
	if (status != STATUS_SUCCESS)
	{
		KLOG(LOG_ERROR, "RecvRequest() failed, status = %d", status);
		return;
	}

	if (request.command != CMD_CONNECT)
	{
		KLOG(LOG_ERROR, "Wrong command : %d (CMD_CONNECT expected)", request.command);
		return;
	}

	response.header.command = CMD_CONNECT;
	response.header.status = DBG_STATUS_SUCCESS;
	response.header.dataSize = 0;
	response.header.context = *context;
	response.data = NULL;

	SendResponse(&response);

	gDbgInitialized = TRUE;

	KLOG(LOG_INFO, "LtDbg connected !");
}

static void WaitForPacket(KeDebugContext * context)
{
	KeDebugRequest request = { 0 };
	KeDebugResponse response = { 0 };
	KeStatus status = STATUS_FAILURE;
	BOOL running = FALSE;

	if (context == NULL)
	{
		KLOG(LOG_ERROR, "Invalid context parameter");
		return;
	}

	while (running == FALSE)
	{
		status = RecvRequest(&request);
		if (status != STATUS_SUCCESS)
		{
			KLOG(LOG_ERROR, "RecvRequest() failed (status = %d)", status);
			return;
		}

		switch (request.command)
		{
		case CMD_STEP:
			DKLOG(LOG_DEBUG, "Step command");
			running = StepCommand(&request, context, &response);
			break;
		case CMD_CONTINUE:
			DKLOG(LOG_DEBUG, "Continue command");
			running = ContinueCommand(&request, context, &response);
			break;
		case CMD_QUIT:
			DKLOG(LOG_DEBUG, "Quit command");
			running = QuitCommand(&request, context, &response);
			break;
		case CMD_REGISTERS:
			DKLOG(LOG_DEBUG, "Registers command");
			running = RegistersCommand(&request, context, &response);
			break;
		case CMD_DISASS:
			DKLOG(LOG_DEBUG, "Disass command");
			running = DisassCommand(&request, context, &response);
			break;
		case CMD_STACK_TRACE:
			DKLOG(LOG_DEBUG, "Stack trace command");
			running = StackTraceCommand(&request, context, &response);
			break;
		case CMD_MEMORY:
			DKLOG(LOG_DEBUG, "Memory command");
			running = MemoryCommand(&request, context, &response);
			break;
		case CMD_BP:
			DKLOG(LOG_DEBUG, "Breakpoint command");
			break;
		default:
			DKLOG(LOG_DEBUG, "Undefined debug command");
			response.header.command = request.command;
			response.header.context = *context;
			response.header.dataSize = 0;
			response.header.status = DBG_STATUS_FAILURE;
			response.data = NULL;
		}

		if (running == FALSE)
		{
			status = SendResponse(&response);

			if (status != STATUS_SUCCESS)
			{
				DKLOG(LOG_DEBUG, "SendResponse() failed with code : %d", status);
			}
		}
		else
		{
			DKLOG(LOG_DEBUG, "Continuing...");
		}

		CleanupKeDebugRequest(&request);
		CleanupKeDebugResponse(&response);
	}
}

static void CleanupKeDebugResponse(KeDebugResponse * response)
{
	if (response == NULL)
	{
		KLOG(LOG_ERROR, "Invalid response parameter");
		return;
	}

	if (response->data == NULL)
	{
		return;
	}

	kfree(response->data);
}

static void CleanupKeDebugRequest(KeDebugRequest * request)
{
	if (request == NULL)
	{
		KLOG(LOG_ERROR, "Invalid request parameter");
		return;
	}

	if (request->param == NULL)
	{
		return;
	}

	kfree(request->param);
}

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

static BOOL StepCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	context->eflags |= TRAP_FLAG_MASK;

	response->header.command = CMD_STEP;
	response->header.context = *context;
	response->header.dataSize = 0;
	response->header.status = DBG_STATUS_SUCCESS;
	response->data = NULL;

	return TRUE;
}

static BOOL ContinueCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	context->eflags &= 0x0FEFF;

	response->header.command = CMD_CONTINUE;
	response->header.context = *context;
	response->header.dataSize = 0;
	response->header.status = DBG_STATUS_SUCCESS;
	response->data = NULL;

	return TRUE;
}

static BOOL QuitCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	context->eflags &= 0x0FEFF;

	response->header.command = CMD_QUIT;
	response->header.context = *context;
	response->header.dataSize = 0;
	response->header.status = DBG_STATUS_SUCCESS;
	response->data = NULL;

	return TRUE;
}

static BOOL RegistersCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	response->header.command = CMD_REGISTERS;
	response->header.context = *context;
	response->header.dataSize = 0;
	response->header.status = DBG_STATUS_SUCCESS;
	response->data = NULL;

	return FALSE;
}

static BOOL DisassCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	KeDebugDisassParamRes * paramRes = NULL;
	KeDebugDisassParamReq * paramReq = NULL;
	unsigned int paramSize = 0;

	response->header.command = CMD_DISASS;
	response->header.context = *context;

	if (request->paramSize == 0 || request->param == NULL)
	{
		KLOG(LOG_ERROR, "DisassCommand() paramSize == 0 or/and param == NULL");
		response->header.status = DBG_STATUS_FAILURE;
		return FALSE;
	}

	paramReq = (KeDebugDisassParamReq *)request->param;
	if (paramReq->nbInst == 0)
	{
		response->header.status = DBG_STATUS_SUCCESS;
		return FALSE;
	}

	paramSize = sizeof(KeDebugDisassParamRes) + (paramReq->nbInst * DEFAULT_ASM_BUFFER_SIZE);
	response->header.dataSize = paramSize;
	response->data = (char *)kmalloc(paramSize);
	if (response->data == NULL)
	{
		KLOG(LOG_ERROR, "DisassCommand(), couldn't allocate memory for response->data");
		response->header.status = DBG_STATUS_FAILURE;
	}

	paramRes = (KeDebugDisassParamRes *)response->data;
	paramRes->size = paramReq->nbInst * DEFAULT_ASM_BUFFER_SIZE;
	paramRes->startingAddress = context->eip;
	MmCopy((void *)context->eip, &paramRes->data, paramRes->size);

	response->header.status = DBG_STATUS_SUCCESS;	

	return FALSE;
}

static BOOL StackTraceCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	u8 * buffer = NULL;
	List * list = ListCreate();
	unsigned int nbPtr = 0;
	u32 * ebp = (u32*)context->ebp;
	unsigned int bufferSize = 0;
	
	ListPush(list, (void *)context->eip);
	nbPtr++;
	
	while (ebp != NULL)
	{
		kprint("ptr : %x\n", (void*)ebp[1]);
		ListPush(list, (void *)ebp[1]);
		ebp = (u32 *)ebp[0];
		nbPtr++;
	}
	
	bufferSize = nbPtr * sizeof(u32);
		
	buffer = (u8 *)kmalloc(bufferSize);
	if (buffer == NULL)
	{
		KLOG(LOG_ERROR, "Can't allocate %d bytes\n", bufferSize);
		goto clean;
	}
	
	unsigned int index = 0;
	unsigned int * addresses = (unsigned int *)buffer;
	while (list != NULL)
	{
		addresses[index++] = (unsigned int)ListPop(&list);
	}

	response->header.command = CMD_STACK_TRACE;
	response->header.context = *context;
	response->header.dataSize = bufferSize;
	response->header.status = DBG_STATUS_SUCCESS;
	response->data = (char *)addresses;

clean:
	if (list != NULL)
	{
		ListDestroy(list);
		list = NULL;
	}

	return FALSE;
}

static BOOL MemoryCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
	unsigned int addr = 0;
	unsigned int size = 0;
	char * buffer = NULL;
	PageDirectoryEntry * currentPd = NULL;

	response->header.command = CMD_MEMORY;
	response->header.context = *context;

	KeDebugMemoryParamReq * reqParam = (KeDebugMemoryParamReq *)request->param;

	if (request->paramSize != sizeof(KeDebugMemoryParamReq))
	{
		KLOG(LOG_ERROR, "Wrong param size");

		response->header.dataSize = 0;
		response->header.status = DBG_STATUS_WRONG_PARAMETER;
		response->data = NULL;
		return FALSE;
	}

	addr = reqParam->startingAddress;
	size = reqParam->nbBytes;

	// Changer de répertoire de table de page (à récupérer dans cr3 ?)
	// Garder en mémoire le courant
	currentPd = _getCurrentPagesDirectory();
	_setCurrentPagesDirectory((PageDirectoryEntry *)context->cr3);
	
	// Vérifier si l'adresse demandée est accessible et renvoyer le résultat 1 ou 0
	if (!IsVirtualAddressAvailable(addr))
	{
		KLOG(LOG_WARNING, "Memory unavailable (0x%x)", addr);

		response->header.dataSize = 0;
		response->header.status = DBG_STATUS_MEMORY_UNAVAILABLE;
		response->data = NULL;

		goto clean;
	}
		
	// Vérifier si l'adresse de fin est également accessible
	if (!IsVirtualAddressAvailable(addr + size - 1))
	{
		KLOG(LOG_WARNING, "Memory unavailable (0x%x)", addr + size - 1);

		response->header.dataSize = 0;
		response->header.status = DBG_STATUS_MEMORY_UNAVAILABLE;
		response->data = NULL;

		goto clean;
	}
	
	buffer = (char *)kmalloc(size);
	if (buffer == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", size);

		response->header.dataSize = 0;
		response->header.status = DBG_STATUS_FAILURE;
		response->data = NULL;

		goto clean;
	}

	MmCopy((void *)addr, buffer, size);

	response->header.status = DBG_STATUS_SUCCESS;
	response->header.dataSize = size;
	response->data = buffer;
	
clean:
	// Restaurer répertoire de pages
	_setCurrentPagesDirectory(currentPd);
	return FALSE;
}

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