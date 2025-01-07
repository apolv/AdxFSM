//$Id: main.cpp 342 2025-01-05 00:40:58Z apolv $
//This demo shows behavior of Events: they are catched by WaitFor() regardless of the time it was sent -- before or after waiting.
//Execution order is the following regardless of inserted random delays: 
//A/A1 (concurrent) -> B -> C/C1 (concurrent) -> D -> A/A1 ->...
//	MyProdTask:						MyConsTask:
//	(A)	Random delay				(A1)Random delay
//	(B)	Send "Produced" event	->	Wait for "Produced" event
//	(C1)Random delay				(C)	Random delay
//	Wait for "Consumed" event	<-	(D)	Send "Consumed" event

#include "core/adx_TimerEvent.h"
using namespace adx_fsm;

void MyProdTask() __attribute__ ((OS_task));
void MyConsTask() __attribute__ ((OS_task));
FSM_t MyProdFsm{ uint32_t(MyProdTask), 64}, MyConsFsm{ uint32_t(MyConsTask), 64};
TimerEvent_t MyProdTimEvt{ &MyProdFsm }, MyConsTimEvt{ &MyConsFsm };
Event_t ProducedEvt{&MyConsFsm}, ConsumedEvt{&MyProdFsm};

uint8_t Random()
{
	static uint8_t x = 1;
	x = (x<<3) - 1 + x;
	return x>>6;
}

void MyProdTask()
{
	while(true)
	{
		SleepFor(&MyProdTimEvt,Random());	//(A)
		ProducedEvt();						//(B)
		SleepFor(&MyProdTimEvt,Random());	//(C1)
		WaitFor(&ConsumedEvt);
	}
}
void MyConsTask()
{
	while(true)
	{
		SleepFor(&MyConsTimEvt,Random());	//(A1)
		WaitFor(&ProducedEvt);
		SleepFor(&MyConsTimEvt,Random());	//(C)
		ConsumedEvt();						//(D)
	}
}

int main(void)
{
	{
		CriticalSection_t cs;
		//Hardware can be initialized here...
		FSM_t::Start();
	}
	
	// Scheduler's Main Loop
	SystemStatus_t Status;
	while (ProcessSystemStatus(Status))
	{
		if (ProcessNextEventInQue())
		{
			//Do something On Idle here...
		}
	}
}
