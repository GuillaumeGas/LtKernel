#pragma once

#include <kernel/init/vmm.h>
#include <kernel/lib/stdlib.h>

#define USER_TASK_P_ADDR 0x100000
#define USER_TASK_V_ADDR 0x400000

//tmp, le temps de mieux gérer la mémoire et permettre l'utilisation d'une liste chaînée
#define NB_MAX_PROCESS 5

struct process
{
	int pid;
	unsigned int start_execution_time;
	struct page_directory_entry * pd;
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
void switch_process(int pid);

void _start_process(struct page_directory_entry * pd);