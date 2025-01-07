//$Id: adx_kernel.h 263 2024-11-11 13:00:52Z apolv $

#if !defined(KERNEL_ADX_KERNEL_H_)
#define KERNEL_ADX_KERNEL_H_

#include "config/adx_kernel_config.h"

#include <stdint.h>

namespace adx_kernel
{
	struct Context_t
	{
		union
		{
			struct  
			{
				uint8_t spl;
				uint8_t sph;
			};
			uint8_t* ptrStackTop;
		};
		uint8_t regs[18];//18 registers
		uint8_t sreg{ 0 };
		volatile uint8_t cs_counter{ 0 };
	} __attribute__((packed));
	
	extern Context_t SystemMainContext;
	extern Context_t* ptrSystemCurrentContext;
	
	void InitTaskContext(uint32_t TaskAddress, Context_t* ptrTaskContext, uint8_t* ptrStackBottom);
	
	extern "C" void SwitchContext(Context_t* ptrCurrentContext, Context_t* ptrNextContext); //Do NOT invoke from ISR!!! //Can be invoked only from SwitchToConext()
	inline void SwitchToContext(Context_t* ptrNextContext)
	{
		Context_t* ptrPrevContext = ptrSystemCurrentContext;
		ptrSystemCurrentContext = ptrNextContext;
		SwitchContext(ptrPrevContext,ptrNextContext);
	}
}

namespace adx_fsm
{
	class CriticalSection_t
	{
		public:
		inline static void Enter()
		{
			//Enter system CS
			asm volatile ("cli" ::);
			adx_kernel::ptrSystemCurrentContext->cs_counter++;
		}
		inline static void Exit()
		{
			switch (adx_kernel::ptrSystemCurrentContext->cs_counter)
			{
			case 0:
				return;
			case 1:
				adx_kernel::ptrSystemCurrentContext->cs_counter--;
				//Exit system CS
				asm volatile ("sei" ::);
				return;
			default:
				adx_kernel::ptrSystemCurrentContext->cs_counter--;
			}
		}
		inline CriticalSection_t() { Enter(); }
		inline ~CriticalSection_t() { Exit(); }
	};
	
	inline void __nop() { asm volatile ("nop" ::); }//Used for debug
}
#endif /* KERNEL_ADX_KERNEL_H_ */