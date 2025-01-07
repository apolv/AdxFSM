//$Id: main.cpp 345 2025-01-06 09:19:22Z apolv $
//This demo shows simple data transfer protocol with packet auto synchronization when some data is lost
//Two Tasks writes their own data packets to UART independently, third Task reads from UART and tries to sync to packet and checks the CheckSum.
//Consistency of Tx packets (when mutual writing to UART) for each writing Task is provided by  OpenWrite()/CloseWrite() methods based on mutex
//To run this demo -- connect Tx pin with Rx pin of the selected UART on your board, otherwise reading Task will be waiting for data forever.
//Writing Tasks do not wait for reading Task, they write packets independently with delay of 1 tick (5ms by default).

#include "drivers/adx_UsartDriver.h"
#include "drivers/adx_SystemClock.h"
#include "core/adx_TimerEvent.h"
using namespace adx_fsm;

ADX_USART(MyUart,USARTE0)

void MyWriteTaskN()  __attribute__ ((OS_task));
void MyReadTask()  __attribute__ ((OS_task));
FSM_t WrFsm_array[] = {{uint32_t(MyWriteTaskN),192}, {uint32_t(MyWriteTaskN),192}};
FSM_t RdFsm{uint32_t(MyReadTask),160};
Event_t MyRdEvt{&RdFsm};

static const uint8_t PackSiganture = 0xaa;
union Packet_t
{
	uint8_t Buf[7];
	struct
	{
		uint8_t Header[2];//= {PackSiganture, CmdId}
		uint8_t Data[4];
		uint8_t CheckSum;
	};
};


uint8_t CalcCheckSum(Packet_t* ptrPack)
{
	uint8_t result = 0;
	for(uint8_t i = 0; i < sizeof(Packet_t)-sizeof(Packet_t::CheckSum); i++) result = result + ptrPack->Buf[i];
	return result-1;
}

void MyWriteTaskN()
{
	TimerEvent_t TimEvt{GetRunningFsm()};
	Event_t MyWrEvt{GetRunningFsm()};
	uint8_t Id = GetRunningFsm()->GetFsmID();
	Packet_t Pack{PackSiganture,Id,static_cast<uint8_t>(Id<<2), static_cast<uint8_t>((Id<<2)+1), static_cast<uint8_t>((Id<<2)+2), static_cast<uint8_t>((Id<<2)+3),0};
	Pack.CheckSum = CalcCheckSum(&Pack);
	while(true)
	{
		auto hUart = MyUart.OpenWrite(&MyWrEvt);
		Write_bl(hUart,Pack.Header,sizeof(Packet_t::Header));
		Write_bl(hUart,Pack.Data,sizeof(Packet_t::Data));
		Write_bl(hUart,&Pack.CheckSum,sizeof(Packet_t::CheckSum));
		MyUart.CloseWrite();
		SleepFor(&TimEvt,1);
	}
}

void MyReadTask()
{
	Packet_t Pack;
	uint8_t CheckSum;
	uint8_t Counter0 = 0, Counter1 = 0;
	
	while(true)
	{
		auto hUart = MyUart.OpenRead_bl(&MyRdEvt);
		Read_bl(hUart,Pack.Buf,sizeof(Packet_t));
		CheckSum = CalcCheckSum(&Pack);
		while(CheckSum != Pack.CheckSum || Pack.Header[0] != PackSiganture)
		{//Search for PackSiganture and try again
			do{
				Read_bl(hUart,Pack.Header,1);
			} while(Pack.Header[0] != PackSiganture);
			Read_bl(hUart,Pack.Buf+1,sizeof(Packet_t)-1);
			CheckSum = CalcCheckSum(&Pack);
			if(CheckSum == Pack.CheckSum) break;
		}
		//Packet below is valid, count received packets with Id==0 and Id==1:
		if(Pack.Header[1] == 0)
			Counter0++;
		else
			Counter1++;
		MyUart.CloseRead();
	}
}

const UsartConfig_t MyCfg PROGMEM = GetConfig(
	32'000'000,
	115'200,
	UsartSettings_t::FrameLen::b8,
	UsartSettings_t::Parity::None,
	UsartSettings_t::StopBits::One,
	UsartSettings_t::RxInterruptPriority::Middle,
	UsartSettings_t::TxInterruptPriority::Middle);

int main(void)
{
	{
		CriticalSection_t cs;
		SetSysAndPerClk_32MHzFrom16MHzXTAL();
		MyUart.Config(&MyCfg);
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

