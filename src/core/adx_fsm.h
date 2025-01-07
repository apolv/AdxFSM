//$Id: adx_fsm.h 346 2025-01-06 16:58:04Z apolv $

#if !defined(CORE_ADX_FSM_H_)
#define CORE_ADX_FSM_H_

#include <stdint.h>

#include "kernel/adx_kernel.h"

namespace adx_fsm
{
	using SignalCounter_t = uint16_t;
	class Signal_t;
	using ptrSignal_t = Signal_t*;
	class FSM_t;
	using ptrFSM_t = FSM_t*;
	using Task_t = void (*)();
	
	class FSM_t
	{
		friend inline ptrSignal_t GetFsmSignal();
		friend inline SignalCounter_t GetFsmCounter();
		friend inline ptrFSM_t GetRunningFsm();
		friend inline void Wait();
	public:
		FSM_t() = delete;
		FSM_t(uint32_t TaskAddress, uint16_t TaskStackSize);
		virtual inline void operator()(ptrSignal_t ptrSignal, SignalCounter_t SignalCounter);
		static void Start();
		inline uint8_t GetFsmID() const {return FsmID;}
	private:
		//FSM List
		ptrFSM_t ptrNextFsm{ nullptr };
		static ptrFSM_t ptrFsmList;
		uint8_t FsmID{ 0 };
	protected:
		//FSM Context
		adx_kernel::Context_t FsmContext{};
		ptrFSM_t ptrPrevFsm { nullptr };
	private:
		//Stack Top End pointer for Stack allocation
		static uint8_t* ptrFsmStackTopEnd;
	protected:	
		//Signal which was used when invoking FSM
		ptrSignal_t ptrFsmSignal { nullptr };
		SignalCounter_t FsmSignalCounter{ 0 };
		static ptrFSM_t ptrRunningFsm; //global FSM pointer for Signal info access from Task via friend functions
	};
	
	inline void FSM_t::operator()(ptrSignal_t ptrSignal, SignalCounter_t SignalCounter)
	{
		ptrFsmSignal = ptrSignal;
		FsmSignalCounter = SignalCounter;
		ptrPrevFsm = ptrRunningFsm;
		ptrRunningFsm = this;
		adx_kernel::SwitchToContext(&FsmContext);
	}
	
	inline ptrSignal_t GetFsmSignal()
	{
		if(FSM_t::ptrRunningFsm != nullptr) return FSM_t::ptrRunningFsm->ptrFsmSignal;
		return nullptr;
	}
	inline SignalCounter_t GetFsmCounter()
	{
		if(FSM_t::ptrRunningFsm != nullptr) return FSM_t::ptrRunningFsm->FsmSignalCounter;
		return 0;
	}
	inline ptrFSM_t GetRunningFsm()
	{
		return FSM_t::ptrRunningFsm;
	}
	inline void Wait()
	{
		if(FSM_t::ptrRunningFsm != nullptr)
		{
			if(FSM_t::ptrRunningFsm->ptrPrevFsm != nullptr)
			{
				FSM_t::ptrRunningFsm = FSM_t::ptrRunningFsm->ptrPrevFsm;
				adx_kernel::SwitchToContext(&(FSM_t::ptrRunningFsm->FsmContext));
			}
			else
			{
				FSM_t::ptrRunningFsm = nullptr;
				adx_kernel::SwitchToContext(&adx_kernel::SystemMainContext);
			}
		}
	}
}

#endif /* CORE_ADX_FSM_H_ */