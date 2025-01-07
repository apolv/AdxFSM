//$Id: adx_Event.cpp 280 2024-11-21 17:21:08Z apolv $

#include "core/adx_Event.h"

using namespace adx_fsm;

SystemQueue_t<LowPrioritySystemEventQueue_SIZE, ptrEvent_t> Event_t::LowPrtyEventQueue;
SystemQueue_t<MiddlePrioritySystemEventQueue_SIZE, ptrEvent_t> Event_t::MiddlePrtyEventQueue;
SystemQueue_t<HighPrioritySystemEventQueue_SIZE, ptrEvent_t> Event_t::HighPrtyEventQueue;

static const uint16_t FileNameHash = CRC16_Modbus("adx_Event.cpp");
//Error and Warning Codes:
static const uint8_t Warning_LowPriorityQueueOverload = 5; //Number of successive calls from Low priority queue exceeded threshold defined in settings
static const uint8_t Warning_MiddlePriorityQueueOverload = 6; //Number of successive calls from Middle priority queue exceeded threshold defined in settings
static const uint8_t Warning_HighPriorityQueueOverload = 7; //Number of successive calls from High priority queue exceeded threshold defined in settings

Event_t::~Event_t()
{
	CriticalSection_t cs;
	if (pptrEventInQueue != nullptr)
	{
		*pptrEventInQueue = nullptr;
		pptrEventInQueue = nullptr;
	}
}

void Event_t::SetMode(Mode_t inMode)
{
	CriticalSection_t cs;
	switch (inMode)
	{
	case Mode_t::Active:
		Mode = Mode_t::Active;
		if ( (Counter > 0) && (pptrEventInQueue == 0) )
		{
			pptrEventInQueue = PutPtrEventInQueue();
		}
		return;
	case Mode_t::Silent:
		Mode = Mode_t::Silent;
		return;
	default:
		Mode = Mode_t::Disabled;
		Counter = 0;
		return;
	}
}

bool adx_fsm::ProcessNextEventInQue()
{
	ptrEvent_t ptrEventFromQueue;
	SignalCounter_t CounterFromQueue;

	//High Priority Queue One Step Processing
	static uint8_t HighPrtyCounter = 0;
	if (HighPrtyCounter < MaxNumberOfSuccessiveHighPrtyCalls)
	{
		do
		{
			CriticalSection_t::Enter();
			if (Event_t::HighPrtyEventQueue.Get(ptrEventFromQueue))
			{
				if (ptrEventFromQueue != 0)
				{
					ptrEventFromQueue->pptrEventInQueue = nullptr;
					if ((ptrEventFromQueue->Mode == SignalMode_t::Active) && (ptrEventFromQueue->Counter > 0))
					{
						CounterFromQueue = ptrEventFromQueue->Counter;
						ptrEventFromQueue->Counter = 0;
						if (ptrEventFromQueue->ptrFsm != nullptr)
						{
							CriticalSection_t::Exit();
							(*(ptrEventFromQueue->ptrFsm))(ptrEventFromQueue, CounterFromQueue);
							HighPrtyCounter++;
							return false;
						}
						else
						{
							CriticalSection_t::Exit();
							continue;
						}
					}
					else
					{
						CriticalSection_t::Exit();
						continue;
					}

				}
				else
				{
					CriticalSection_t::Exit();
					continue;
				}
			}
			else
			{
				CriticalSection_t::Exit();
				break;
			}
		} while (true);
	}
	else
	{
		Warning(Warning_HighPriorityQueueOverload, static_cast<uint16_t>(__LINE__), FileNameHash, Signal_t::ErrorHandler);
	}
	HighPrtyCounter = 0;

	//Middle Priority Queue One Step Processing
	static uint8_t MiddlePrtyCounter = 0;
	if (MiddlePrtyCounter < MaxNumberOfSuccessiveMiddlePrtyCalls)
	{
		do
		{
			CriticalSection_t::Enter();
			if (Event_t::MiddlePrtyEventQueue.Get(ptrEventFromQueue))
			{
				if (ptrEventFromQueue != 0)
				{
					ptrEventFromQueue->pptrEventInQueue = nullptr;
					if ((ptrEventFromQueue->Mode == SignalMode_t::Active) && (ptrEventFromQueue->Counter > 0))
					{
						CounterFromQueue = ptrEventFromQueue->Counter;
						ptrEventFromQueue->Counter = 0;
						if (ptrEventFromQueue->ptrFsm != nullptr)
						{
							CriticalSection_t::Exit();
							(*(ptrEventFromQueue->ptrFsm))(ptrEventFromQueue, CounterFromQueue);
							MiddlePrtyCounter++;
							return false;
						}
						else
						{
							CriticalSection_t::Exit();
							continue;
						}
					}
					else
					{
						CriticalSection_t::Exit();
						continue;
					}
				}
				else
				{
					CriticalSection_t::Exit();
					continue;
				}
			}
			else
			{
				CriticalSection_t::Exit();
				break;
			}
		} while (true);
	}
	else
	{
		Warning(Warning_MiddlePriorityQueueOverload, static_cast<uint16_t>(__LINE__), FileNameHash, Signal_t::ErrorHandler);
	}
	MiddlePrtyCounter = 0;

	//Low Priority Queue One Step Processing
	static uint8_t LowPrtyCounter = 0;
	if (LowPrtyCounter < MaxNumberOfSuccessiveLowPrtyCalls)
	{
		do
		{
			CriticalSection_t::Enter();
			if (Event_t::LowPrtyEventQueue.Get(ptrEventFromQueue))
			{
				if (ptrEventFromQueue != 0)
				{
					ptrEventFromQueue->pptrEventInQueue = nullptr;
					if ((ptrEventFromQueue->Mode == SignalMode_t::Active) && (ptrEventFromQueue->Counter > 0))
					{
						CounterFromQueue = ptrEventFromQueue->Counter;
						ptrEventFromQueue->Counter = 0;
						if (ptrEventFromQueue->ptrFsm != nullptr)
						{
							CriticalSection_t::Exit();
							(*(ptrEventFromQueue->ptrFsm))(ptrEventFromQueue, CounterFromQueue);
							LowPrtyCounter++;
							return false;
						}
						else
						{
							CriticalSection_t::Exit();
							continue;
						}
					}
					else
					{
						CriticalSection_t::Exit();
						continue;
					}
				}
				else
				{
					CriticalSection_t::Exit();
					continue;
				}
			}
			else
			{
				CriticalSection_t::Exit();
				break;
			}
		} while (true);
	}
	else
	{
		Warning(Warning_LowPriorityQueueOverload, static_cast<uint16_t>(__LINE__), FileNameHash, Signal_t::ErrorHandler);
	}
	LowPrtyCounter = 0;
	
	//Exit to Idle Process
	return true;
}