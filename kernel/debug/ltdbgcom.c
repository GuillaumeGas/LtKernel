#include "ltdbgcom.h"

#include <kernel/drivers/serial.h>
#include <kernel/lib/kmalloc.h>
#include <kernel/logger.h>
#include <kernel/lib/stdio.h>

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)

u8 ReadByte()
{
	return SerialRead(COM2_PORT);
}

void ReadBytes(u8 * buffer, unsigned int size)
{
	if (buffer == NULL)
		return;

	for (int i = 0; i < size; i++)
	{
		buffer[i] = ReadByte();
	}
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

	ReadBytes((u8 *)&(packet->size), sizeof(unsigned int));

	KLOG(LOG_DEBUG, "Reading %d bytes...", packet->size);

	if (packet->size == 0)
		return STATUS_SUCCESS;

	packet->content = (u8 *)kmalloc(packet->size);
	if (packet->content == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", packet->size);
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
		KLOG(LOG_ERROR, "Invalid packet parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (packet->content == NULL)
	{
		KLOG(LOG_ERROR, "packet->content is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	if (packet->size == 0)
	{
		KLOG(LOG_ERROR, "packet->size == 0");
		return STATUS_INVALID_PARAMETER;
	}

	WriteBytes((u8 *)&packet->size, sizeof(unsigned int));
	WriteBytes(packet->content, packet->size);

	return STATUS_SUCCESS;
}

void CleanupPacket(KeDebugPacket * packet)
{
	if (packet == NULL)
	{
		KLOG(LOG_ERROR, "Invalid packet parameter");
		return;
	}

	if (packet->content != NULL)
	{
		kfree(packet->content);
		packet->content = NULL;
	}
}

KeStatus RecvRequest(KeDebugRequest * request)
{
	KeDebugPacket packet = { 0 };
	KeDebugRequest * ptrRequest = NULL;
	KeStatus status = STATUS_FAILURE;

	status = RecvPacket(&packet);
	if (status != STATUS_SUCCESS)
	{
		KLOG(LOG_ERROR, "RecvPacket() failed with status : %d", status);
		return status;
	}

	if (packet.size == 0 || packet.content == NULL)
	{
		KLOG(LOG_DEBUG, "packet.size == %d || packet.content == %x", packet.size, packet.content);
		return STATUS_SUCCESS;
	}

	ptrRequest = (KeDebugRequest *)packet.content;
	request->command = ptrRequest->command;
	request->paramSize = ptrRequest->paramSize;

	request->param = (char *)kmalloc(request->paramSize);
	if (request->param == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes for request->param", request->paramSize);
		return STATUS_ALLOC_FAILED;
	}

	MmCopy(&(ptrRequest->param), request->param, request->paramSize);

	CleanupPacket(&packet);

	return STATUS_SUCCESS;
}

KeStatus SendResponse(KeDebugResponse * response)
{
	KeDebugPacket packet;
	u8 * buffer = NULL;

	if (response == NULL)
	{
		KLOG(LOG_ERROR, "Invalid response parameter");
		return STATUS_NULL_PARAMETER;
	}

	if (response->header.dataSize != 0 && response->data == NULL)
	{
		KLOG(LOG_ERROR, "response->data shouldn't be NULL (dataSize : %d)", response->header.dataSize);
		return STATUS_INVALID_PARAMETER;
	}

	packet.size = sizeof(KeDebugResponseHeader) + response->header.dataSize;

	buffer = (u8 *)kmalloc(packet.size);
	if (buffer == NULL)
	{
		KLOG(LOG_ERROR, "Couldn't allocate %d bytes", packet.size);
		return STATUS_ALLOC_FAILED;
	}

	MmCopy(&response->header, buffer, sizeof(KeDebugResponseHeader));
	MmCopy(response->data, buffer + sizeof(KeDebugResponseHeader), response->header.dataSize);

	packet.content = buffer;

	return SendPacket(&packet);
}