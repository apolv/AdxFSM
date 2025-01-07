//$Id: adx_SystemQueue.h 273 2024-11-15 08:53:09Z apolv $

#if !defined (CORE_ADX_FSM_SYSTEM_QUEUE_H_)
#define CORE_ADX_FSM_SYSTEM_QUEUE_H_

#include <stdint.h>

namespace adx_fsm
{
	template <uint8_t SIZE = 16, typename T = uint8_t>
	class SystemQueue_t
	{
		static_assert(1 <= SIZE, "SystemQueue SIZE should be >=1");
		static_assert(SIZE <= 128, "SystemQueue SIZE should be <=128");
		static_assert(((SIZE) & ((uint8_t)(SIZE - 1))) == 0, "SystemQueue SIZE should be a power of 2");
	private:
		T data_array[SIZE];
		volatile uint8_t PutIndex{ 0 };
		volatile uint8_t GetIndex{ 0 };
		static const uint8_t Mask = SIZE - 1;
	public:
		inline bool IsNotFull() const { return (uint8_t(PutIndex - GetIndex) & uint8_t(~Mask)) == 0; }
		inline bool IsNotEmpty() const { return PutIndex != GetIndex; }
		inline bool Get(T& rData)
		{
			if (IsNotEmpty())
			{
				rData = data_array[GetIndex & Mask];
				//Enter Atomic
				GetIndex++;
				//Exit Atomic
				return true;
			}
			return false;
		}
		inline T* PutE(T Data)
		{
			if (IsNotFull())
			{
				uint8_t currIndex = PutIndex & Mask;
				data_array[currIndex] = Data;
				//Enter Atomic
				PutIndex++;
				//Exit Atomic
				return (data_array + currIndex);
			}
			return nullptr;
		}
		inline bool Put(const T& Data)
		{
			if (IsNotFull())
			{
				data_array[PutIndex & Mask] = Data;
				//Enter Atomic
				PutIndex++;
				//Exit Atomic
				return true;
			}
			return false;
		}
	};
}
#endif // !ADX_FSM_SYSTEM_QUEUE_H