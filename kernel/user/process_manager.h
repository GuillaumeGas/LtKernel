#pragma once

#include <kernel/init/memory.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/types.h>

#define USER_TASK_P_ADDR 0x100000
#define USER_TASK_V_ADDR 0x400000

#define USER_STACK_START_ADDR 0x400A00

// Sélecteurs de segment de code et de pile d'une tâche utilisateur
// Les bits RPL (niveau de privilège) sont à '11' afin de permettre le passage en niveau de privilèges utilisateur
// lors de l'exécution de l'instruction iret (voir process_starter.asm et le fonctionnement de iret)
#define USER_CODE_SEG_SELECTOR 0x1B
#define USER_STACK_SEG_SELECTOR 0x23

//tmp, le temps de mieux gérer la mémoire et permettre l'utilisation d'une liste chaînée
#define NB_MAX_PROCESS 5

struct process
{
	int pid;
	unsigned int start_execution_time;
	struct page_directory_entry * pd;
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

enum ExecMode { KERNEL, USER };
typedef enum ExecMode ExecMode;

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
void create_process(u8 * task_addr, unsigned int size);
void start_process(int pid);

void _start_process(struct page_directory_entry * pd, u32 ss, u32 esp, u32 eflags, u32 cs, u32 eip,
	u32 eax, u32 ecx, u32 edx, u32 ebx, u32 ebp, u32 esi, u32 edi, u32 ds, u32 es, u32 fs, u32 gs,
	ExecMode execMode);