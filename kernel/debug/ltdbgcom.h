#pragma once

#include "ltdbg.h"

#include <kernel/lib/stdlib.h>

u8 ReadByte();
void ReadBytes(u8 * buffer, unsigned int size);
void WriteByte(u8 byte);
void WriteBytes(u8 * buffer, unsigned int bufferSize);

KeStatus RecvPacket(KeDebugPacket * packet);
KeStatus SendPacket(KeDebugPacket * packet);
KeStatus RecvRequest(KeDebugRequest * request);
KeStatus SendResponse(KeDebugResponse * response);