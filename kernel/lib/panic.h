#pragma once

enum PanicType { HEAP_LIMIT, MEMORY_FULL };
typedef enum PanicType PanicType;

void panic(PanicType type);