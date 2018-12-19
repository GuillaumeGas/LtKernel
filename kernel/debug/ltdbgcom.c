#include "ltdbgcom.h"

#include <kernel/drivers/serial.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/logger.h>

u8 ReadByte()
{
	return SerialRead(COM2_PORT);
}

void ReadBytes(u8 * buffer, unsigned int size)
{
	if (buffer == NULL)
		return;

	for (int i = 0; i < size; i++)
		buffer[i] = ReadByte();
}

void WriteByte(u8 byte)
{
	SerialWrite(COM2_PORT, byte);
}

void WriteBytes(u8 * buffer, unsigned int bufferSize)
{
	for (unsigned int i = 0; i < bufferSize; i++)
	{
		WriteByte(buffer[i]);
	}
}

KeStatus RecvPacket(KeDebugPacket * packet)
{
	if (packet == NULL)
	{
		return STATUS_NULL_PARAMETER;
	}

	ReadBytes((u8 *)&packet->size, sizeof(unsigned int));

	if (packet->size == 0)
		return STATUS_SUCCESS;

	packet->content = (u8 *)kmalloc(packet->size);
	if (packet->content == NULL)
	{
		// TODO : logger
		packet->size = 0;
		return STATUS_ALLOC_FAILED;
	}

	ReadBytes(packet->content, packet->size);

	return STATUS_SUCCESS;
}

KeStatus SendPacket(KeDebugPacket * packet)
{	
	if (packet == NULL)
	{
		// TODO : logger
		return STATUS_NULL_PARAMETER;
	}

	if (packet->content == NULL || packet->size == 0)
	{
		// TODO : logger
		return STATUS_INVALID_PARAMETER;
	}

	WriteBytes((u8 *)packet->size, sizeof(unsigned int));
	WriteBytes(packet->content, packet->size);

	return STATUS_SUCCESS;
}

KeStatus RecvRequest(KeDebugRequest * request)
{
	KeDebugPacket packet = { 0 };
	KeStatus status = STATUS_FAILURE;

	status = RecvPacket(&packet);
	if (status != STATUS_SUCCESS)
	{
		// TODO : logger
		return status;
	}

	if (packet.size == 0 || packet.content == NULL)
	{
		// TODO : logger
		return STATUS_SUCCESS;
	}

	if (packet.size != sizeof(KeDebugRequest))
	{
		// TODO : logger, paquet mal formé
		return STATUS_UNEXPECTED;
	}

	MmCopy(packet.content, (u8 *)request, packet.size);

	return STATUS_SUCCESS;
}

KeStatus SendResponse(KeDebugResponse * response)
{
	KeDebugPacket packet;
	u8 * buffer = NULL;

	if (response == NULL)
	{
		// TODO : logger
		return STATUS_NULL_PARAMETER;
	}

	if (response->header.dataSize != 0 && response->data == NULL)
	{
		//TODO : logger
		return STATUS_INVALID_PARAMETER;
	}

	packet.size = sizeof(KeDebugResponseHeader) + response->header.dataSize;

	buffer = (u8 *)kmalloc(packet.size);
	if (buffer == NULL)
	{
		//TODO : logger
		return STATUS_ALLOC_FAILED;
	}

	MmCopy((u8 *)&response->header, buffer, sizeof(KeDebugResponseHeader));
	MmCopy((u8 *)response->data, buffer + sizeof(KeDebugResponseHeader), response->header.dataSize);

	packet.content = buffer;

	return SendPacket(&packet);
}