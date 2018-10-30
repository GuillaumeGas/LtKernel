#include "process.h"

#include <kernel/user/process_manager.h>

Process * GetCurrentProcess()
{
    return gCurrentProcess;
}