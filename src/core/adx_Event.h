//$Id: adx_Event.h 341 2025-01-04 14:31:57Z apolv $

#if !defined (CORE_ADX_EVENT_H_)
#define CORE_ADX_EVENT_H_

#include "core/adx_Signal.h"
#include "core/adx_SystemQueue.h"
#include "config/adx_core_config.h"

namespace adx_fsm
{
	class Event_t;
	using ptrEvent_t = Event_t*;

	class Event_t: public Signal_t
	{
		friend bool ProcessNextEventInQue();
	public:
		enum class Priority_t : uint8_t
		{
			Low = 0,
			Middle,
			High
		};
		Event_t() = delete;
		Event_t(ptrFSM_t ptrFsmFunctor, Mode_t initMode = Mode_t::Silent, Priority_t initPriority = Priority_t::Middle) :
			Signal_t(ptrFsmFunctor, initMode), Priority(initPriority) {}
		~Event_t();
		virtual void SetMode(Mode_t)  override;
		virtual inline bool Send() override final;
		void SetPriority(Priority_t InPriority) { Priority = InPriority; }
		Priority_t GetPriority() const { return Priority; }
		inline bool operator()() override final { return Send(); }
	private:
		ptrEvent_t* volatile pptrEventInQueue{ nullptr };
		Priority_t Priority{ Priority_t::Middle };
		static SystemQueue_t<LowPrioritySystemEventQueue_SIZE, ptrEvent_t> LowPrtyEventQueue;
		static SystemQueue_t<MiddlePrioritySystemEventQueue_SIZE, ptrEvent_t> MiddlePrtyEventQueue;
		static SystemQueue_t<HighPrioritySystemEventQueue_SIZE, ptrEvent_t> HighPrtyEventQueue;

		inline ptrEvent_t* PutPtrEventInQueue();
	};

	using EventPrty_t = Event_t::Priority_t;

	//Error and Warning Codes:
	const uint8_t Warning_EventCounterOverflow = 1; //Event Counter tries to exceed maximum of the used integral type
	const uint8_t Error_LowPriorityQueueOverflow = 2; //Number of Events put in the queue exceeds Low Priority queue SIZE, defined in settings
	const uint8_t Error_MiddlePriorityQueueOverflow = 3; //Number of Events put in the queue exceeds Middle Priority queue SIZE, defined in settings
	const uint8_t Error_HighPriorityQueueOverflow = 4; //Number of Events put in the queue exceeds High Priority queue SIZE, defined in settings
	
	const uint16_t AdxEventH_FileNameHash = CRC16_Modbus("adx_Event.h");
	
	//Inline class methods:
	inline ptrEvent_t* Event_t::PutPtrEventInQueue()
	{
		ptrEvent_t* PutResult;
		switch (Priority)
		{
		case Priority_t::Low:
			PutResult = LowPrtyEventQueue.PutE(this);
			if (PutResult == nullptr)
			{
				Error(Error_LowPriorityQueueOverflow, static_cast<uint16_t>(__LINE__), AdxEventH_FileNameHash, ErrorHandler);
			}
			return PutResult;
		case Priority_t::Middle:
			PutResult = MiddlePrtyEventQueue.PutE(this);
			if (PutResult == nullptr)
			{
				Error(Error_MiddlePriorityQueueOverflow, static_cast<uint16_t>(__LINE__), AdxEventH_FileNameHash, ErrorHandler);
			}
			return PutResult;
		default: //Priority_t::High
			PutResult = HighPrtyEventQueue.PutE(this);
			if (PutResult == nullptr)
			{
				Error(Error_HighPriorityQueueOverflow, static_cast<uint16_t>(__LINE__), AdxEventH_FileNameHash, ErrorHandler);
			}
		}
		return PutResult;
	}

	inline bool Event_t::Send()
	{
		CriticalSection_t cs;
		switch (Mode)
		{
		case Mode_t::Disabled:
			return false;
		case Mode_t::Active:
			if (pptrEventInQueue == nullptr)
			{
				pptrEventInQueue = PutPtrEventInQueue();
			}
		default: //Mode_t::Silent, returns "true"
			if (Counter < SignalCounter_t(~0U))
			{
				Counter++;
			}
			else
			{
				Warning(Warning_EventCounterOverflow, static_cast<uint16_t>(__LINE__), AdxEventH_FileNameHash, ErrorHandler);
			}
			return true;
		}
	}

	//Global functions:
	bool ProcessNextEventInQue();
}

#endif // !ADX_EVENT_H_
