//$Id: adx_DataSignal.h 273 2024-11-15 08:53:09Z apolv $

#if !defined(CORE_ADX_DATA_SIGNAL_H_)
#define CORE_ADX_DATA_SIGNAL_H_

#include "core/adx_Signal.h"
#include "core/adx_SaE_Data.h"

namespace adx_fsm
{
	template <typename T>
	class DataSignal_t : public Signal_t, public SaE_Data_t<T>
	{
	public:
		DataSignal_t() = delete;
		DataSignal_t(const T& rinitData, ptrFSM_t ptrFsm, Mode_t initMode = Mode_t::Disabled) :
			Signal_t(ptrFsm, initMode),
			SaE_Data_t<T>(rinitData) {}
	};
}
#endif