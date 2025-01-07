//$Id: main.cpp 277 2024-11-19 00:31:27Z apolv $
/*Signals and Events example*/
// 1) TaskA calls direct context switching to TaskB via Signal()
// 2) TaskB sends Event to TaskA then returns to caller context (TaskA)
// 3) TaskA returns to caller context (main())
// 4) Sheduler in maim() gets Event (sent by TaskB) and calls context switching to TaskA (i.e. goto step (1))
// Data attached to Signal won't loss until Signal is active (in this case SignalCounter == 1)
// Data attached to Event always consistent but is lost when SignalCounter > 1

#include <avr/io.h>

#include "core/adx_DataEvent.h"
#include "core/adx_DataSignal.h"

using namespace adx_fsm;

void TaskA() __attribute__ ((OS_task));
void TaskB() __attribute__ ((OS_task));

FSM_t fsm1(uint32_t(TaskA),64);
FSM_t fsm2(uint32_t(TaskB),64);
DataEvent_t<uint8_t> evf1(1, &fsm1,SignalMode_t::Active);
DataSignal_t<uint8_t> sigf2(2, &fsm2,SignalMode_t::Active);

void TaskA()
{
	uint8_t a = 0;
	while(true)
	{
		if(GetFsmSignal() == &evf1) evf1.GetData(a);
		a++;
		sigf2.SetData(a);
		sigf2();
		a++;
		WaitFor(&evf1);
		//Wait();
	}
}

void TaskB()
{
	uint8_t b = 0;
	while(true)
	{
		if(GetFsmSignal() == &sigf2) sigf2.GetData(b);
		b++;
		evf1.SetData(b);
		evf1();
		b++;
		Wait();
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

