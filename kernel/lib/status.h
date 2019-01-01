#pragma once

enum KeStatus
{
	/* General */
	STATUS_FAILURE,
	STATUS_SUCCESS,
	STATUS_UNEXPECTED,

	/* Parameters */
	STATUS_INVALID_PARAMETER,
	STATUS_NULL_PARAMETER,

	/* Allocation */
	STATUS_ALLOC_FAILED,

	/* Processes */
	STATUS_PROCESS_NOT_FOUND,

	/* Elf files */
	STATUS_NOT_ELF_FILE,

	/* Virtual memory */
	STATUS_INVALID_VIRTUAL_USER_ADDRESS,

	/* Physical memory */
	STATUS_PHYSICAL_MEMORY_FULL,
} typedef KeStatus;

#define FAILED(status) (status != STATUS_SUCCESS)