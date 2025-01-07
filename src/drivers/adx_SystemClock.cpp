//$Id: adx_SystemClock.cpp 274 2024-11-15 11:35:28Z apolv $

#include "drivers/adx_SystemClock.h"

#include <avr/io.h>

#include "kernel/adx_kernel.h"

using namespace adx_fsm;

void adx_fsm::CCPWrite( volatile uint8_t * address, uint8_t value )
{
	volatile uint8_t * tmpAddr = address;
	#ifdef RAMPZ
	RAMPZ = 0;
	#endif
	
	CriticalSection_t cs;
	asm volatile(
	"movw r30,  %0"	      "\n\t"
	"ldi  r16,  %2"	      "\n\t"
	"out   %3, r16"	      "\n\t"
	"st     Z,  %1"       "\n\t"
	:
	: "r" (tmpAddr), "r" (value), "M" (0xd8), "i" (&CCP)
	: "r16", "r30", "r31"
	);//0xd8 = CCP_IOREG_gc
}

void adx_fsm::SetSysAndPerClk_32MHzFrom16MHzXTAL()
{
	// Configure External Oscillator
	OSC.XOSCCTRL = (OSC_FRQRANGE_12TO16_gc)|(0 << OSC_X32KLPM_bp)|(OSC_XOSCSEL_XTAL_1KCLK_gc);

	// Start External Oscillator
	OSC.CTRL |= OSC_XOSCEN_bm;
	do {} while ((OSC.STATUS & (OSC_XOSCRDY_bm)) == 0);

	// Configure PLL to run at 4x of Ext Osc
	OSC.PLLCTRL = (OSC_PLLSRC_XOSC_gc)|(4 << OSC_PLLFAC_gp);

	// Prescalers Configuration: CLKper = CLKcpu = 32MHz
	uint8_t PSconfig = (uint8_t) CLK_PSADIV_2_gc | CLK_PSBCDIV_1_1_gc;
	CCPWrite(&CLK.PSCTRL, PSconfig);

	// Start PLL and turn off all other clock sources
	OSC.CTRL = OSC_XOSCEN_bm | OSC_PLLEN_bm;
	do {} while ((OSC.STATUS & (OSC_PLLRDY_bm)) == 0);

	// Main Clock Source Select
	uint8_t clkCtrl = (CLK.CTRL & ~CLK_SCLKSEL_gm) | CLK_SCLKSEL_PLL_gc;
	CCPWrite(&CLK.CTRL, clkCtrl);
}