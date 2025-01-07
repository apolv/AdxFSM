//$Id: adx_kernel.cpp 263 2024-11-11 13:00:52Z apolv $

#include "kernel/adx_kernel.h"

#include <stdint.h>

using namespace adx_kernel;

void adx_kernel::InitTaskContext(uint32_t TaskAddress, Context_t* ptrTaskContext, uint8_t* ptrStackBottom)
{
	union FarAddr_t
	{
		uint32_t Addr;
		uint8_t byte[4];
	} ptrTask;
	
	ptrTask.Addr = TaskAddress;

	ptrTaskContext->ptrStackTop = ptrStackBottom;
	*(ptrTaskContext->ptrStackTop) = ptrTask.byte[0];
	ptrTaskContext->ptrStackTop--;
	*(ptrTaskContext->ptrStackTop) = ptrTask.byte[1];
	ptrTaskContext->ptrStackTop--;
	*(ptrTaskContext->ptrStackTop) = ptrTask.byte[2];
	ptrTaskContext->ptrStackTop--;
}

Context_t adx_kernel::SystemMainContext{};
Context_t* adx_kernel::ptrSystemCurrentContext = &SystemMainContext;
