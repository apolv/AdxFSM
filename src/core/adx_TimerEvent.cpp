//$Id: adx_TimerEvent.cpp 346 2025-01-06 16:58:04Z apolv $

#include "core/adx_TimerEvent.h"

using namespace adx_fsm;

uint8_t TimerEvent_t::NofTimerEvents = 0;
uint8_t TimerEvent_t::NofNotDisabledTimerEvents = 0;
TimerEvent_t* TimerEvent_t::TimerPtrArray[SystemTimerEventBuffer_SIZE];
volatile uint16_t TimerEvent_t::GlobalTimerCounter = 0;

//Error and Warning Codes:
static const uint8_t Error_SystemTimerEventBufferOverflow = 8; //Number of TimerEvent_t objects is greater than SystemTimerEventBuffer_SIZE
static const uint16_t FileNameHash = CRC16_Modbus("adx_TimerEvent.cpp");

TimerEvent_t::TimerEvent_t(ptrFSM_t ptrFsmFunctor, Mode_t initMode, Priority_t initPriority):
	Event_t(ptrFsmFunctor,initMode,initPriority)
{
	if(NofTimerEvents >= SystemTimerEventBuffer_SIZE) 
	{
		Error(Error_SystemTimerEventBufferOverflow, static_cast<uint16_t>(__LINE__), FileNameHash, ErrorHandler);
		return;
	}
	TimerPtrArray[NofTimerEvents] = this;
	if(NofTimerEvents == 0) ConfigSystemTimer();
	NofTimerEvents++;
}

void TimerEvent_t::ConfigSystemTimer()
{
	ADX_SYSTEM_TIMER_BASE.CTRLA |= SystemTimerFperPrescaler;
	ADX_SYSTEM_TIMER_BASE.PERBUF = SystemTimerPeriod;
	ADX_SYSTEM_TIMER_BASE.CTRLFSET = (0x01<<2);//Update command
	//ADX_SYSTEM_TIMER_BASE.INTCTRLA |= SystemTimerInterruptLevel; //Turns on Counter Interrupt
	PMIC.CTRL |= SystemTimerPmicBitMask; //Turns on System Timer PMIC Interrupts based on settings
}

inline void TimerEvent_t::TimerISR()
{
	TimerEvent_t* ptrTimEvt;
	for(uint8_t i = 0; i < NofTimerEvents; i++)
	{
		ptrTimEvt = TimerPtrArray[i];
		if(ptrTimEvt->GetMode() != SignalMode_t::Disabled)
		{
			if(ptrTimEvt->TimerCounter == GlobalTimerCounter)
			{
				ptrTimEvt->TimerCounter = GlobalTimerCounter + ptrTimEvt->TimerPeriod + 1;
				ptrTimEvt->Send();
			}
		}
	}
	GlobalTimerCounter++;
}

ISR(ADX_SYSTEM_TIMER_VECTOR)
{
	TimerEvent_t::TimerISR();
}

void TimerEvent_t::SetMode(SignalMode_t TimEvtMode)
{
	CriticalSection_t cs;
	if(Mode == Mode_t::Disabled)
	{
		if(TimEvtMode != SignalMode_t::Disabled)
		{
			TimerEvent_t::NofNotDisabledTimerEvents++;
			TimerCounter = TimerEvent_t::GlobalTimerCounter + TimerPeriod;
			ADX_SYSTEM_TIMER_BASE.INTCTRLA |= SystemTimerInterruptLevel; //Turns on Counter Interrupt
		}
	}
	else
	{
		if(TimEvtMode == SignalMode_t::Disabled)
		{
			if(TimerEvent_t::NofNotDisabledTimerEvents > 0) TimerEvent_t::NofNotDisabledTimerEvents--;
		}
	}
	Event_t::SetMode(TimEvtMode);
	if(TimerEvent_t::NofNotDisabledTimerEvents == 0) ADX_SYSTEM_TIMER_BASE.INTCTRLA &= 0b1111'1100; //Turns off Counter Interrupt
}

void adx_fsm::SleepFor(TimerEvent_t* ptrTimerEvent, uint16_t NofTicks)
{
	if(NofTicks == 0)
	{
		ptrTimerEvent->SetMode(SignalMode_t::Disabled);
		ptrTimerEvent->SetPeriod(0);
		ptrTimerEvent->SetMode(SignalMode_t::Active);
		ptrTimerEvent->Send();
	}
	else
	{
		ptrTimerEvent->SetMode(SignalMode_t::Disabled);
		ptrTimerEvent->SetPeriod(NofTicks-1);
		ptrTimerEvent->SetMode(SignalMode_t::Active);
	}
	Wait();
	ptrTimerEvent->SetMode(SignalMode_t::Disabled);
}

