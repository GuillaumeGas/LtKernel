#pragma once

#include <kernel/init/vmm.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/types.h>
#include <kernel/lib/list.h>

struct process
{
	int pid;
	unsigned int start_execution_time;
	struct page_directory page_directory;
	u32 * kstack_esp0;
	
	struct
	{
		u32 eax, ebx, ecx, edx;
		u32 ebp, esp, esi, edi;
		u32 eip, eflags;
		u32 cs : 16, ss : 16, ds : 16, es : 16, fs : 16, gs : 16;
		u32 cr3;
	} regs;
};
typedef struct process Process;

enum ExecMode { KERNEL, USER };
typedef enum ExecMode ExecMode;

#ifdef __PROCESS_MANAGER__
Process * g_current_process = NULL;
List * g_process_list = NULL;
unsigned int g_nb_process = 0;
#else
extern Process * g_current_process;
extern List * g_process_list;
extern unsigned int g_nb_process;
#endif

void init_process_manager();
void create_process(void * task_addr, unsigned int size);
void start_process(int pid);

void _start_process(PageDirectoryEntry * pd, u32 ss, u32 esp, u32 eflags, u32 cs, u32 eip,
	u32 eax, u32 ecx, u32 edx, u32 ebx, u32 ebp, u32 esi, u32 edi, u32 ds, u32 es, u32 fs, u32 gs,
	ExecMode execMode);