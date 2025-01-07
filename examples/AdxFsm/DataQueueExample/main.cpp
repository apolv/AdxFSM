//$Id: main.cpp 262 2024-11-11 12:48:36Z apolv $
/*Data transfer example:               TaskA->DataQueueAB->TaskB->DataQueueBC->TaskC*/
//Notification between tasks and FIFOs:  via signal  via event via event via signal

#include "core/adx_Event.h"
#include "core/adx_DataQueue.h"

using namespace adx_fsm;

void TaskA() __attribute__ ((OS_task)); //Data source, always wants to write
void TaskB() __attribute__ ((OS_task)); //Data pump from AB FIFO to BC FIFO, always wants to read AND/OR write
void TaskC() __attribute__ ((OS_task)); //Data receiver, always wants to read

//Create FSMs and connect tasks to FSMs
FSM_t FsmA(uint32_t(TaskA),128);
FSM_t FsmB(uint32_t(TaskB),80);
FSM_t FsmC(uint32_t(TaskC),128);

//Create signals/events and connect to FSMs
Signal_t SigA(&FsmA, SignalMode_t::Active);
Event_t EvtB(&FsmB, SignalMode_t::Active, EventPrty_t::Middle);
Signal_t SigC(&FsmC, SignalMode_t::Active);

//Create FIFOs and connect put&get sides to signals/events
DataQueue_t<ptrSignal_t, ptrSignal_t, 8, int> DataQueueAB(&SigA, &EvtB);
DataQueue_t<ptrSignal_t, ptrSignal_t, 8, int> DataQueueBC(&EvtB, &SigC);



void TaskA()
{
	int Data[17] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	while (true)
	{
		Write_bl(&DataQueueAB,Data,17);
	}
}

void TaskB()
{
	int Data;
	while (true)
	{
		Get_bl(&DataQueueAB,Data);
		Put_bl(&DataQueueBC,Data);
	}
}

void TaskC()
{
	int Data[5];
	while (true)
	{
		Read_bl(&DataQueueBC,Data,5);
	}
}

int main(void)
{
	{//Enter CriticalSection
		CriticalSection_t cs;
		//Place Hardware Initialization Code here...
		Event_t::SetErrorHandler([](const SystemStatus_t& rStatus) { __nop(); }); //Connect empty Error Handler for Event system for debug purpose
		FSM_t::Start();
	}//Exit CriticalSection

	SystemStatus_t Status;
	while (ProcessSystemStatus(Status))
	{
		if (ProcessNextEventInQue())
		{
			//Do something On Idle here...
			__nop();//nop is for debug only
		}
	}
}

