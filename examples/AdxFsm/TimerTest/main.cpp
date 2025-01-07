//$Id: main.cpp 341 2025-01-04 14:31:57Z apolv $

#include <avr/io.h>

#include "core/adx_TimerEvent.h"
#include "drivers/adx_SystemClock.h"
using namespace adx_fsm;


void MyTask() __attribute__ ((OS_task));
FSM_t MyFsm{ uint32_t(MyTask), 256};
TimerEvent_t MyTimerEvent{ &MyFsm };
	
void MyTask()
{
	volatile uint16_t MyCounter = 0;
	PORTB.DIRSET = PIN0_bm;
	while(true)
	{
		MyTimerEvent.SetPeriod(199);
		MyTimerEvent.SetMode(SignalMode_t::Active);
		Wait();
		MyTimerEvent.SetMode(SignalMode_t::Silent);
		
		MyCounter += GetFsmCounter();
		PORTB.OUTTGL = PIN0_bm;
	}
}

int main(void)
{
	{//Enter CriticalSection
		CriticalSection_t cs;
		SetSysAndPerClk_32MHzFrom16MHzXTAL();
		
		///////////////// Start of User Initialization Code //////////////////////////
		Event_t::SetErrorHandler([](const SystemStatus_t& rStatus) { __nop(); }); //Connect empty Error Handler for Event system for debug purpose
		///////////////// End of User Initialization Code //////////////////////////
		
		FSM_t::Start();
	}//Exit CriticalSection
	
	// Scheduler's Main Loop
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
