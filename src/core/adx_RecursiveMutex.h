//$Id: adx_RecursiveMutex.h 279 2024-11-19 22:40:57Z apolv $

#if !defined(CORE_ADX_RECURSIVE_MUTEX_H_)
#define CORE_ADX_RECURSIVE_MUTEX_H_

#include "config/adx_core_config.h"
#include "core/adx_Signal.h"
#include "core/adx_SystemQueue.h"

namespace adx_fsm
{
	class RecursiveMutex_t
	{
	public:
		inline bool TryLock(ptrSignal_t);
		inline void Lock_bl(ptrSignal_t);
		inline void Unlock();
		RecursiveMutex_t() {}
		RecursiveMutex_t(const RecursiveMutex_t&) = delete;
		RecursiveMutex_t& operator=(const RecursiveMutex_t&) = delete;
	private:
		SystemQueue_t<RecursiveMutexSignalQueue_SIZE, ptrSignal_t> SignalQueue;
		ptrSignal_t ptrOwnersSignal{ nullptr };
		ptrSignal_t ptrWaitingSignal{ nullptr };
		volatile uint8_t Counter{ 0 };
	};
	
	inline void RecursiveMutex_t::Lock_bl(ptrSignal_t ptrSig)
	{
		ptrSig->SetMode(SignalMode_t::Active);
		while (!TryLock(ptrSig)) Wait();
		ptrSig->SetMode(SignalMode_t::Silent);
	}

	inline bool RecursiveMutex_t::TryLock(ptrSignal_t inPtrSignal) //inPtrSignal must != nullptr
	{
		CriticalSection_t cs;
		if ((ptrWaitingSignal == nullptr) || (inPtrSignal == ptrWaitingSignal))
		{
			if (ptrOwnersSignal == nullptr)
			{
				ptrOwnersSignal = inPtrSignal;
				Counter++;
				return true;
			}
			else
			{
				if (ptrOwnersSignal->GetFsm() == inPtrSignal->GetFsm())
				{
					Counter++;
					return true;
				}
				else
				{
					if (!SignalQueue.Put(inPtrSignal)) { inPtrSignal->Send(); }
					return false;
				}
			}
		}
		else
		{
			if (!SignalQueue.Put(inPtrSignal)) { inPtrSignal->Send(); }
			return false;
		}
	}

	inline void RecursiveMutex_t::Unlock()
	{
		CriticalSection_t cs;
		switch (Counter)
		{
		case 0U:
			return;
		case 1U:
			Counter--;
			if (SignalQueue.Get(ptrWaitingSignal))
			{
				ptrOwnersSignal = nullptr;
				ptrWaitingSignal->Send();
				return;
			}
			else //when SignalQueue is empty
			{
				ptrOwnersSignal = nullptr;
				ptrWaitingSignal = nullptr;
				return;
			}
		default:
			Counter--;
		}
		return;
	}
}

#endif