//$Id: adx_SaE_Data.h 273 2024-11-15 08:53:09Z apolv $

#if !defined(CORE_ADX_SAE_DATA_H_)
#define CORE_ADX_SAE_DATA_H_

#include "kernel/adx_kernel.h"

namespace adx_fsm
{
	template< typename T >
	class SaE_Data_t
	{
	private:
		T Data;
	public:
		SaE_Data_t() = delete;
		SaE_Data_t(const T& rInitData): Data(rInitData){}
		inline void GetData(T&); //Attention!!! Output Data is consistent, but overwritten by last not processed event.
		inline void SetData(const T&);
	};
	
	//Complete Template Specialization <uint8_t> is used for fast Set()/Get() call w/o critical section
	template<>
	class SaE_Data_t<uint8_t>
	{
	protected:
		uint8_t Data;
	public:
		SaE_Data_t() = delete;
		SaE_Data_t(const uint8_t& rInitData): Data(rInitData){}
		inline void GetData(uint8_t& rOutData) {rOutData = Data;}
		inline void SetData(uint8_t InData) {Data = InData;}
	};
	
	//Implementation
	template< typename T >
	inline void SaE_Data_t<T>::GetData(T& rOutData)
	{
		CriticalSection_t cs;
		rOutData = Data;
	}
	
	template< typename T >
	inline void SaE_Data_t<T>::SetData(const T& rInData)
	{
		CriticalSection_t cs;
		Data = rInData;
	}
	
}

#endif