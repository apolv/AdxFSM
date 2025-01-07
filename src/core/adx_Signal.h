//$Id: adx_Signal.h 346 2025-01-06 16:58:04Z apolv $

#if !defined(CORE_ADX_SIGNAL_H_)
#define CORE_ADX_SIGNAL_H_

#include <stdint.h>
#include "kernel/adx_kernel.h"
#include "core/adx_fsm.h"
#include "core/adx_SystemStatus.h"

namespace adx_fsm
{
	class Signal_t
	{
	public:
		enum class Mode_t : uint8_t
		{
			Disabled = 0,
			Silent,
			Active = 3
		};
	protected:
		ptrFSM_t const ptrFsm;
		volatile SignalCounter_t Counter{ 0 };
		volatile Mode_t Mode{ Mode_t::Disabled };
		static ErrorHandler_t ErrorHandler;
	public:
		Signal_t() = delete;
		Signal_t(ptrFSM_t initPtrFsm, Mode_t initMode = Mode_t::Silent) : ptrFsm(initPtrFsm), Mode(initMode) {}
		virtual inline bool Send();
		virtual inline bool operator()() {return Send();}
		virtual void SetMode(Mode_t);
		inline Mode_t GetMode() const { return Mode; }
		inline ptrFSM_t GetFsm() const { return ptrFsm; }
		static void SetErrorHandler(ErrorHandler_t InErrorHandler) { ErrorHandler = InErrorHandler; }
	};

	using SignalMode_t = Signal_t::Mode_t;

	//Error and Warning Codes:
	const uint8_t Warning_SignalCounterOverflow = 1; //Signal Counter tries to exceed maximum of the used integral type
	const uint16_t AdxSignalH_FileNameHash = CRC16_Modbus("adx_Signal.h");
	
	inline bool Signal_t::Send() 
	{
		SignalCounter_t cnt;
		CriticalSection_t::Enter();
		switch (Mode)
		{
		case Mode_t::Active:
			if (Counter < SignalCounter_t(~0U))
			{
				Counter++;
			}
			else
			{
				Warning(Warning_SignalCounterOverflow, static_cast<uint16_t>(__LINE__), AdxSignalH_FileNameHash, ErrorHandler);
			}
			if(ptrFsm != GetRunningFsm()) //check for next fsm != current fsm
			{
				cnt = Counter;
				Counter = 0;
				CriticalSection_t::Exit();
				(*ptrFsm)(this, cnt);
				return true;
			}
			CriticalSection_t::Exit();
			return false;
		case Mode_t::Silent:
			if (Counter < SignalCounter_t(~0U))
			{
				Counter++;
			}
			else
			{
				Warning(Warning_SignalCounterOverflow, static_cast<uint16_t>(__LINE__), AdxSignalH_FileNameHash, ErrorHandler);
			}
		default: //Mode_t::Disabled
			CriticalSection_t::Exit();
			return false;
		}
	}
	
	inline void WaitFor(ptrSignal_t ptrWaitingSignal)
	{
		ptrWaitingSignal->SetMode(SignalMode_t::Active);
		Wait();
		ptrWaitingSignal->SetMode(SignalMode_t::Silent);
	}
}

#endif