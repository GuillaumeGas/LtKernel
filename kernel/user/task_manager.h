#pragma once

#include <kernel/init/vmm.h>

#define USER_TASK_P_ADDR 0x100000
#define USER_TASK_V_ADDR 0x400000

struct page_directory_entry * create_task();
void switch_task(struct page_directory_entry * pd);