//$Id: adx_DataEvent.h 273 2024-11-15 08:53:09Z apolv $

#if !defined(CORE_ADX_DATA_EVENT_H_)
#define CORE_ADX_DATA_EVENT_H_

#include "core/adx_Event.h"
#include "core/adx_SaE_Data.h"

namespace adx_fsm
{
	template <typename T>
	class DataEvent_t : public Event_t, public SaE_Data_t<T>
	{
	public:
		DataEvent_t() = delete;
		DataEvent_t(const T& rinitData, ptrFSM_t ptrFsm, Mode_t initMode = Mode_t::Disabled, Priority_t initPriority = Priority_t::Middle) :
			Event_t(ptrFsm, initMode, initPriority),
			SaE_Data_t<T>(rinitData) {}
	};
}
#endif