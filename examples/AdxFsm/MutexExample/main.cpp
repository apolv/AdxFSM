//$Id: main.cpp 324 2024-12-19 23:23:40Z apolv $
/*Recursive mutex example*/
//Three tasks (with MyID = 1, 3, 5) tries to lock the same mutex for different number of scheduler cycles (number of cycles == MyID value)
//As a result task with MyID == 5, gets lock for 5 cycles, then task with MyID == 3 - for 3 cycles, then task with MyID == 1 - for 1 cycle
//Then all repeated

#include "core/adx_Fsm.h"
#include "core/adx_Event.h"
#include "core/adx_RecursiveMutex.h"

using namespace adx_fsm;

void MyTaskN() __attribute__ ((OS_task));

//Deriving New class from FSM_t is a way to pass parameters to Task and to avoid static variables in tasks
class MyFsm_t : public FSM_t
{
	friend void MyTaskN();
public:
	MyFsm_t(uint8_t initMyID) : FSM_t(uint32_t(MyTaskN),128), MyID(initMyID) {}
private:
	Event_t MyEvent{ this };
	const uint8_t MyID{ 0 };
	static RecursiveMutex_t mx;
};
RecursiveMutex_t MyFsm_t::mx;

MyFsm_t fsm_array[3] = {1,3,5};

void MyFunction_bl(RecursiveMutex_t& mx, ptrSignal_t ptrSig, uint8_t NofDelays)
{
	ptrSig->SetMode(SignalMode_t::Active);
	while (!mx.TryLock(ptrSig)) Wait(); //Example of mutex TryLock (call w/o blocking)
	ptrSig->SetMode(SignalMode_t::Silent);
	
	for(uint8_t i = 0; i < NofDelays; i++)
	{
		ptrSig->Send();
		WaitFor(ptrSig);
	}
	mx.Unlock();
}

void MyTaskN()
{
	//Creating aliases for convenient Fsm member access
	MyFsm_t* ptrFsm = (MyFsm_t*)(GetRunningFsm());
	RecursiveMutex_t& mx = ptrFsm->mx;
	const uint8_t& MyID = ptrFsm->MyID;
	Signal_t& MyEvent = ptrFsm->MyEvent;
	while (true)
	{
		mx.Lock_bl(&MyEvent); //Example of mutex lock with blocking
		MyFunction_bl(mx, &MyEvent, MyID);
		mx.Unlock();
	}
}



int main()
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