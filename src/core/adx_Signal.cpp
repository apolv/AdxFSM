//$Id: adx_Signal.cpp 278 2024-11-19 12:08:04Z apolv $

#include "core/adx_Signal.h"

using namespace adx_fsm;

ErrorHandler_t Signal_t::ErrorHandler{ nullptr };
	
void Signal_t::SetMode(Mode_t inMode)
{
	switch (inMode)
	{
	case Mode_t::Active:
		Mode = Mode_t::Active; //cs is not required because Mode is one byte
		return;
	case Mode_t::Silent:
		Mode = Mode_t::Silent; //cs is not required because Mode is one byte
		return;
	default: //inMode == Disabled
		CriticalSection_t::Enter();
		Mode = Mode_t::Disabled;
		Counter = 0;
		CriticalSection_t::Exit();
		return;
	}
}