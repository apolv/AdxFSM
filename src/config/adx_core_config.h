//$Id: adx_core_config.h 347 2025-01-06 22:53:39Z apolv $

/*Configuration for core/xxx source files*/

#if !defined (CONFIG_ADX_CORE_CONFIG_H_)
#define CONFIG_ADX_CORE_CONFIG_H_

#include <stdint.h>

namespace adx_fsm
{
	//Event System Config:
	const uint8_t LowPrioritySystemEventQueue_SIZE = 4;
	const uint8_t MiddlePrioritySystemEventQueue_SIZE = 8;
	const uint8_t HighPrioritySystemEventQueue_SIZE = 4;

	const uint8_t MaxNumberOfSuccessiveLowPrtyCalls = 4;
	const uint8_t MaxNumberOfSuccessiveMiddlePrtyCalls = 8;
	const uint8_t MaxNumberOfSuccessiveHighPrtyCalls = 16;

	//Mutex Config:
	const uint8_t RecursiveMutexSignalQueue_SIZE = 4;
	
	//TimerEvent_t system timer Config:
	const uint8_t SystemTimerEventBuffer_SIZE = 8; //Max Number of not disabled timer events, not greater than N of FSMW_t objects
	//#define ADX_SYSTEM_TIMER_TCxx //default is TCF0 if not defined
	//uncomment and replace xx with C0,C1,D0,D1,E0,E1 for another timer
	//#define ADX_SYSTEM_TIMER_INT_xxx //default is middle priority
	//uncomment and replace xxx with HIGH or LOW for another priority
	//System timer Prescaler:
	//x1 = 0x01,
	//div1 = 0x02,
	//div4 = 0x03,
	//div8 = 0x04,
	//div64 = 0x05,
	//div256 = 0x06,
	//div1024 = 0x07
	const uint8_t SystemTimerFperPrescaler = 0x06;
	//System timer period:
	//Time resolution, i.e. ISR Period = (Prescaler * (Period + 1))/Fper;
	const uint16_t SystemTimerPeriod = 624; //per624, div256 -> 5ms at Fper==32MHz

}

#endif // !CONFIG_ADX_CORE_CONFIG_H_


