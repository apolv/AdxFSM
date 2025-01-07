//$Id: main.cpp 262 2024-11-11 12:48:36Z apolv $
/*Example of direct context switching using fsm(), i.e. oprator()*/
//fsm(ptrSignal,Counter) calls direct context switching to task connected to Signal
//Wait() returns to previous context (to the context, which has called the current context)

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "core/adx_fsm.h"

using namespace adx_fsm;

int foo(int x)
{
	int y = 0;
	y = x + GetFsmCounter();
	Wait();
	return y;
}


//__attribute__((section(".mysection_name"))) //use this attribute to place Task above 64k FLASH, setup section address (.mysection_name=0xAABBCCDD) in linker settings
__attribute__ ((OS_task))
void TaskB()
{
	volatile int a = 0;
	while(true)
	{
		{
			CriticalSection_t cs;
			asm volatile("nop"::);
		}
		a = foo(a);
	}
}

FSM_t fsm1(uint32_t(TaskB),64);
FSM_t fsm2(uint32_t(TaskB),64);

__attribute__ ((OS_task))
void TaskA()
{
	volatile int a = 100;
	while(true)
	{
		asm volatile("nop"::);
		{
			CriticalSection_t cs;
			a++;
			fsm1(GetFsmSignal(),GetFsmCounter());
		}
		Wait();
	}
}

FSM_t fsm0(uint32_t(TaskA),64);


int main(void)
{
	FSM_t::Start();
	volatile int a = 10;
	while(true)
	{
		asm volatile("nop"::);
		{
			CriticalSection_t cs;
			a++;
			fsm0(nullptr,1);
		}
		fsm2(nullptr,5);
	}
}
