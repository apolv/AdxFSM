//$Id: main.cpp 279 2024-11-19 22:40:57Z apolv $

#include "drivers/adx_UsartDriver.h"
#include "drivers/adx_SystemClock.h"
using namespace adx_fsm;

//....................... User Declarations and Definitions ..................
ADX_USART(MyUsart,USARTE0)

void MyTask() __attribute__ ((OS_task));
FSM_t MyFsm{ uint32_t(MyTask), 256 };
Event_t MyEvent{ &MyFsm };
	
void MyTask()
{
	uint8_t DataBuf[5];
	while(true)
	{
		auto ptrReadObject = MyUsart.OpenRead_bl(&MyEvent);
		auto ptrWriteObject = MyUsart.OpenWrite_bl(&MyEvent);
		Read_bl(ptrReadObject,DataBuf,5);
		Write_bl(ptrWriteObject,DataBuf,5);
		MyUsart.CloseWrite();
		MyUsart.CloseRead();
	}
	
}

const UsartConfig_t MyCfg PROGMEM = GetConfig(
	32'000'000,
	921'600,
	UsartSettings_t::FrameLen::b8,
	UsartSettings_t::Parity::None,
	UsartSettings_t::StopBits::One,
	UsartSettings_t::RxInterruptPriority::Middle,
	UsartSettings_t::TxInterruptPriority::Middle);

int main(void)
{
	{//Enter CriticalSection
		CriticalSection_t cs;
		SetSysAndPerClk_32MHzFrom16MHzXTAL();
		
		///////////////// Start of User Initialization Code //////////////////////////
		Event_t::SetErrorHandler([](const SystemStatus_t& rStatus) { __nop(); }); //Connect empty Error Handler for Event system for debug purpose
		UsartDriver_t::SetErrorHandler([](const SystemStatus_t& rStatus) { __nop(); }); //Connect empty Error Handler for Usarts for debug purpose
		MyUsart.Config(&MyCfg);
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
