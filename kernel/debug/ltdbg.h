#pragma once

#include <kernel/lib/types.h>

#include "ltdbgcommands.h"

#define DEFAULT_ASM_BUFFER_SIZE 20

enum BpState
{
	BP_ENABLED,
	BP_DISABLED
} typedef BpState;

enum KeDebugStatus
{
	DBG_STATUS_SUCCESS,
	DBG_STATUS_FAILURE,
	DBG_STATUS_ALREADY_CONNECTED,
	DBG_STATUS_BREAKPOINT_REACHED,
} typedef KeDebugStatus;

struct KeDebugContext
{
	u32 cr3, cr2, cr0;
	u32 gs, fs, es, ds, ss;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 eip;
	u16 cs;
	u32 eflags;
} typedef KeDebugContext;

struct KeBreakpoint
{
	u32 addr;
	BpState state;
	u8 savedInstByte;
	int id;
} typedef KeBreakpoint;

struct KeDebugPacket
{
	unsigned int size;
	u8 * content; // KeDebugRequest ou KeDebugResponse
} typedef KeDebugPacket;

struct KeDebugRequest
{
	CommandId command;
	unsigned int paramSize;
	char * param;
} typedef KeDebugRequest;

struct KeDebugResponseHeader
{
	CommandId command;
	KeDebugStatus status;
	KeDebugContext context;
	unsigned int dataSize;
} typedef KeDebugResponseHeader;

struct KeDebugResponse
{
	KeDebugResponseHeader header;
	char * data;
} typedef KeDebugResponse;

/* DISASS CMD */
struct KeDebugDisassParamReq
{
	unsigned int nbInst;
} typedef KeDebugDisassParamReq;

struct KeDebugDisassParamRes
{
	unsigned int size;
	unsigned int startingAddress;
	char * data;
} typedef KeDebugDisassParamRes;
/* DISASS CMD */

void DbgInit();