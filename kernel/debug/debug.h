#pragma once

#include <kernel/lib/types.h>
#include <kernel/init/isr.h>

void kdump ();
void panicStr (const char * str);
void PrintInterruptContext(InterruptContext * context);
void PrintExceptionContext(ExceptionContext * context);
void PrintExceptionContextWithCode(ExceptionContextWithCode * context);
