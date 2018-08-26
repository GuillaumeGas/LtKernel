#pragma once

#include <kernel/init/vmm.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/types.h>

#define USER_TASK_P_ADDR 0x100000
#define USER_TASK_V_ADDR 0x400000

#define USER_STACK_START_ADDR 0x400A00

// S�lecteurs de segment de code et de pile d'une t�che utilisateur
// Les bits RPL (niveau de privil�ge) sont � '11' afin de permettre le passage en niveau de privil�ges utilisateur
// lors de l'ex�cution de l'instruction iret (voir process_starter.asm et le fonctionnement de iret)
#define USER_CODE_SEG_SELECTOR 0x1B
#define USER_STACK_SEG_SELECTOR 0x23

//tmp, le temps de mieux g�rer la m�moire et permettre l'utilisation d'une liste cha�n�e
#define NB_MAX_PROCESS 5

struct process
{
	int pid;
	unsigned int start_execution_time;
	struct page_directory_entry * pd;
	
	struct
	{
		u32 eax, ebx, ecx, edx;
		u32 ebp, esp, esi, edi;
		u32 eip, eflags;
		u32 cs : 16, ss : 16, ds : 16, es : 16, fs : 16, gs : 16;
		u32 cr3;
	} regs;
};

#ifdef __PROCESS_MANAGER__
struct process * g_current_process = NULL;
struct process g_process_list[NB_MAX_PROCESS];
unsigned int g_nb_process = 0;
#else
extern struct process * g_current_process;
extern struct process g_process_list[];
extern unsigned int g_nb_process;
#endif

void init_process_manager();
void create_task(u8 * task_addr, int size);
void start_process(int pid);

void _start_process(struct page_directory_entry * pd, u32 ss, u32 esp, u32 eflags, u32 cs, u32 eip);