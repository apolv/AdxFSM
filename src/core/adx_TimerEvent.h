//$Id: adx_TimerEvent.h 341 2025-01-04 14:31:57Z apolv $

#if !defined(CORE_ADX_FSMW_H_)
#define CORE_ADX_FSMW_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "core/adx_Event.h"

namespace adx_fsm
{
	class TimerEvent_t: public Event_t
	{
	public:
		TimerEvent_t() = delete;
		TimerEvent_t(ptrFSM_t ptrFsmFunctor, Mode_t initMode = Mode_t::Disabled, Priority_t initPriority = Priority_t::Middle);
		virtual void SetMode(Mode_t)  override final;
		inline void SetPeriod(uint16_t Period);
		static inline void TimerISR();
	private:
		static void ConfigSystemTimer();
		//Local System Timer Data
		uint16_t TimerPeriod{ 0 };
		uint16_t TimerCounter{ 0 };
		//General System Timer Data
		static uint8_t NofTimerEvents;
		static uint8_t NofNotDisabledTimerEvents;
		volatile static uint16_t GlobalTimerCounter;
		static TimerEvent_t* TimerPtrArray[];
	};
	
	inline void TimerEvent_t::SetPeriod(uint16_t Period)
	{
		CriticalSection_t cs;
		TimerPeriod = Period;
	}
	
	//Free functions
	void SleepFor(TimerEvent_t* ptrTimerEvent, uint16_t NofTicks);

	//System timer interrupt level selection depending on config in adx_core_config.h
	#if defined(ADX_SYSTEM_TIMER_INT_HIGH)
		constexpr uint8_t SystemTimerInterruptLevel = 0x03;
		constexpr uint8_t SystemTimerPmicBitMask = 0x04;
	#elif defined(ADX_SYSTEM_TIMER_INT_LOW)
		constexpr uint8_t SystemTimerInterruptLevel = 0x01;
		constexpr uint8_t SystemTimerPmicBitMask = 0x01;
	#else
		constexpr uint8_t SystemTimerInterruptLevel = 0x02;
		constexpr uint8_t SystemTimerPmicBitMask = 0x02;
	#endif
}

//System timer selection depending on config in adx_core_config.h
#if defined(ADX_SYSTEM_TIMER_TCC0)
	#define ADX_SYSTEM_TIMER_VECTOR TCC0_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCC0
#elif defined(ADX_SYSTEM_TIMER_TCC1)
	#define ADX_SYSTEM_TIMER_VECTOR TCC1_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCC1
#elif defined(ADX_SYSTEM_TIMER_TCD0)
	#define ADX_SYSTEM_TIMER_VECTOR TCD0_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCD0
#elif defined(ADX_SYSTEM_TIMER_TCD1)
	#define ADX_SYSTEM_TIMER_VECTOR TCD1_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCD1
#elif defined(ADX_SYSTEM_TIMER_TCE0)
	#define ADX_SYSTEM_TIMER_VECTOR TCE0_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCE0
#elif defined(ADX_SYSTEM_TIMER_TCE1)
	#define ADX_SYSTEM_TIMER_VECTOR TCE1_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCE1
#else
	#define ADX_SYSTEM_TIMER_VECTOR TCF0_OVF_vect
	#define ADX_SYSTEM_TIMER_BASE TCF0
#endif

#endif //CORE_ADX_FSMW_H_