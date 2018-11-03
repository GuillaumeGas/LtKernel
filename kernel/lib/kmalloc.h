#pragma once

#include <kernel/init/vmm.h>

MemBlock * ksbrk(int n);
void * kmalloc(int size);
void kfree(void * ptr);
void dumpHeap();

Page PageAlloc();
void PageFree(void * ptr);