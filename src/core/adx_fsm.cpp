//$Id: adx_fsm.cpp 340 2025-01-03 01:10:29Z apolv $

#include "core/adx_fsm.h"

using namespace adx_fsm;

ptrFSM_t FSM_t::ptrFsmList{ nullptr };
ptrFSM_t FSM_t::ptrRunningFsm{ nullptr };
uint8_t* FSM_t::ptrFsmStackTopEnd = nullptr;
	
FSM_t::FSM_t(uint32_t TaskAddress, uint16_t TaskStackSize)
{
	if (ptrFsmList != nullptr)
	{
		FsmID = ptrFsmList->FsmID + 1;
		ptrNextFsm = ptrFsmList;
	}
	else
	{
		ptrFsmStackTopEnd = adx_kernel::ptrSystemMainStackBottom - adx_kernel::SystemMainStack_SIZE;
	}
	ptrFsmList = this;
	
	adx_kernel::InitTaskContext(TaskAddress, &FsmContext, ptrFsmStackTopEnd);
	ptrFsmStackTopEnd = ptrFsmStackTopEnd - TaskStackSize;
}

void FSM_t::Start()
{
	ptrFSM_t ptrFsm = FSM_t::ptrFsmList;
	while (ptrFsm != nullptr)
	{
		(*ptrFsm)(nullptr,0);
		ptrFsm = ptrFsm->ptrNextFsm;
	}
}