//$Id: adx_SystemClock.h 274 2024-11-15 11:35:28Z apolv $

#if !defined(DRIVERS_ADX_SYSTEM_CLOCK_H_)
#define DRIVERS_ADX_SYSTEM_CLOCK_H_

#include <stdint.h>

namespace adx_fsm
{
	void CCPWrite( volatile uint8_t* address, uint8_t value);
	void SetSysAndPerClk_32MHzFrom16MHzXTAL();
}

#endif