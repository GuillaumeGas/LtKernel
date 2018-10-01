#pragma once

enum PanicType { HEAP_LIMIT, MEMORY_FULL, VIRTUAL_MEMORY_FULL, PAGE_TABLE_NOTE_FOUND };
typedef enum PanicType PanicType;

void panic(PanicType type);