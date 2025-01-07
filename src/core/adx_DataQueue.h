//$Id: adx_DataQueue.h 296 2024-11-29 09:46:05Z apolv $

#if !defined(CORE_ADX_DATA_QUEUE_H_)
#define CORE_ADX_DATA_QUEUE_H_

#include <stdint.h>
#include "core/adx_Event.h"

namespace adx_fsm
{
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE = 16, typename DataT = uint8_t>
	class DataQueue_t
	{
		static_assert(1 <= SIZE, "DataQueue SIZE should be >=1");
		static_assert(SIZE <= 128, "DataQueue SIZE should be <=128");
		static_assert(((SIZE)& ((uint8_t)(SIZE - 1))) == 0, "DataQueue SIZE should be a power of 2");
	private:
		//Queue core:
		DataT data_array[SIZE];
		volatile uint8_t PutIndex{ 0 };
		volatile uint8_t GetIndex{ 0 };
		static const uint8_t Mask = SIZE - 1;
		//Callback mechanics:
		ptrPutsideFunctorT ptrPutsideFunctor{ nullptr };
		ptrGetsideFunctorT ptrGetsideFunctor{ nullptr };
		bool PutsideWakeupFlag{ false };
		volatile uint8_t SizeRemainToWrite{ 0 };
		bool GetsideWakeupFlag{ false };
		volatile uint8_t SizeRemainToRead{ 0 };
		//Helper methods:
		inline void CheckPutsideWakeupCondition(uint8_t);
		inline void ProcPutsideWakeupCondition();
		inline void CheckGetsideWakeupCondition(uint8_t);
		inline void ProcGetsideWakeupCondition();
	public:
		inline DataQueue_t(ptrPutsideFunctorT initPtrPutsideFunctor, ptrGetsideFunctorT initPtrGetsideFunctor, bool initPutsideWakeupFlag = false, bool initGetsideWakeupFlag = false) :
			ptrPutsideFunctor(initPtrPutsideFunctor),
			ptrGetsideFunctor(initPtrGetsideFunctor),
			PutsideWakeupFlag(initPutsideWakeupFlag),
			GetsideWakeupFlag(initGetsideWakeupFlag) {}
		inline void ConnectPutsideFunctor(ptrPutsideFunctorT, bool);
		inline void ConnectGetsideFunctor(ptrGetsideFunctorT, bool);
		inline ptrPutsideFunctorT GetPutsidePtrFunctor() const { return ptrPutsideFunctor; }
		inline ptrGetsideFunctorT GetGetsidePtrFunctor() const { return ptrGetsideFunctor; }
		inline bool IsNotFull() const { return (uint8_t(PutIndex - GetIndex) & uint8_t(~Mask)) == 0; }
		inline bool IsNotEmpty() const { return PutIndex != GetIndex; }
		inline bool Put(const DataT& rData);
		bool Write(DataT*, uint8_t, uint8_t&);
		inline bool Get(DataT& rData);
		bool Read(DataT*, uint8_t, uint8_t&);
	};

	//Interface implementation:
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::ConnectPutsideFunctor(ptrPutsideFunctorT inPtrPutsideFunctor, bool inPutsideWakeupFlag)
	{
		CriticalSection_t cs;
		SizeRemainToWrite = 0;
		PutsideWakeupFlag = inPutsideWakeupFlag;
		ptrPutsideFunctor = inPtrPutsideFunctor;
	}
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::ConnectGetsideFunctor(ptrGetsideFunctorT inPtrGetsideFunctor, bool inGetsideWakeupFlag)
	{
		CriticalSection_t cs;
		SizeRemainToRead = 0;
		GetsideWakeupFlag = inGetsideWakeupFlag;
		ptrGetsideFunctor = inPtrGetsideFunctor;
	}

	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline bool DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::Put(const DataT& rData)
	{
		CheckGetsideWakeupCondition(1U);
		if (IsNotFull())
		{
			data_array[PutIndex & Mask] = rData;
			{ //Enter Atomic
				//CriticalSection_t cs; //Uncomment if PutIndex type size is more than one byte
				PutIndex++;
			} //Exit Atomic
			ProcGetsideWakeupCondition();
			return true;
		}
		else
		{
			if (ptrPutsideFunctor != nullptr)
			{
				SizeRemainToWrite = 1;
			}
			ProcGetsideWakeupCondition();
			return false;
		}
	}

	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	bool DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::
		Write(DataT* ptrData, uint8_t inSize, uint8_t& rinoutSizeWritten)
	{
		CheckGetsideWakeupCondition(inSize - rinoutSizeWritten);
		while (rinoutSizeWritten < inSize)
		{
			if (IsNotFull())
			{
				data_array[PutIndex & Mask] = ptrData[rinoutSizeWritten];
				rinoutSizeWritten++;
				{ //Enter Atomic
					//CriticalSection_t cs; //Uncomment if PutIndex type size is more than one byte
					PutIndex++;
				} //Exit Atomic
			}
			else
			{
				if (ptrPutsideFunctor != nullptr)
				{
					SizeRemainToWrite = inSize - rinoutSizeWritten;
				}
				ProcGetsideWakeupCondition();
				return false;
			}
		}
		ProcGetsideWakeupCondition();
		return true;
	}

	template <class ptrPutsideFunctorT, class ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline bool DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::Get(DataT& rData)
	{
		CheckPutsideWakeupCondition(1U);
		if (IsNotEmpty())
		{
			rData = data_array[GetIndex & Mask];
			{ //Enter Atomic
				//CriticalSection_t cs; //Uncomment if PutIndex type size is more than one byte
				GetIndex++;
			} //Exit Atomic
			ProcPutsideWakeupCondition();
			return true;
		}
		else
		{
			if (ptrGetsideFunctor != nullptr)
			{
				SizeRemainToRead = 1;
			}
			ProcPutsideWakeupCondition();
			return false;
		}
	}

	template <class ptrPutsideFunctorT, class ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	bool DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::
		Read(DataT* ptrData, uint8_t inSize, uint8_t& rinoutSizeRed)
	{
		CheckPutsideWakeupCondition(inSize - rinoutSizeRed);
		while (rinoutSizeRed < inSize)
		{
			if (IsNotEmpty())
			{
				ptrData[rinoutSizeRed] = data_array[GetIndex & Mask];
				rinoutSizeRed++;
				{ //Enter Atomic
					//CriticalSection_t cs; //Uncomment if PutIndex type size is more than one byte
					GetIndex++;
				} //Exit Atomic
			}
			else
			{
				if (ptrGetsideFunctor != nullptr)
				{
					SizeRemainToRead = inSize - rinoutSizeRed;
				}
				ProcPutsideWakeupCondition();
				return false;
			}
		}
		ProcPutsideWakeupCondition();
		return true;
	}

	//Helpers:
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::CheckGetsideWakeupCondition(uint8_t RemainToWrSize)
	{
		if (ptrGetsideFunctor != nullptr)
		{
			if (SizeRemainToRead > 0)
			{
				uint8_t currSize = PutIndex - GetIndex;
				if ((SizeRemainToRead <= (currSize + RemainToWrSize)) || (currSize >= SIZE / 2)) //WakeUp Getside if data in buf will be enough for it after current put/write operation or buf is more than half-full
				{
					GetsideWakeupFlag = true;
					SizeRemainToRead = 0;
				}
			}
		}
	}
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::ProcGetsideWakeupCondition()
	{
		if (ptrGetsideFunctor != nullptr) 
		{
			if (GetsideWakeupFlag)
			{
				GetsideWakeupFlag = !(*ptrGetsideFunctor)();//Sends Event to Getside Task or calls Getside callback function or Functor to WakeUp Getside.
			}
		}
	}
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::CheckPutsideWakeupCondition(uint8_t RemainToRdSize)
	{
		if (ptrPutsideFunctor != nullptr) 
		{
			if (SizeRemainToWrite > 0)
			{
				uint8_t currSize = PutIndex - GetIndex;
				if ((SizeRemainToWrite <= (SIZE - currSize + RemainToRdSize)) || (currSize <= SIZE / 2)) //WakeUp Putside if free space in buf will be enough for it after current get/read operation or buf is more than half-empty
				{
					PutsideWakeupFlag = true;
					SizeRemainToWrite = 0;
				}
			}
		}
	}
	template <typename ptrPutsideFunctorT, typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void DataQueue_t<ptrPutsideFunctorT, ptrGetsideFunctorT, SIZE, DataT>::ProcPutsideWakeupCondition()
	{
		if (ptrPutsideFunctor != nullptr)
		{
			if (PutsideWakeupFlag)
			{
				PutsideWakeupFlag = !(*ptrPutsideFunctor)();//Sends Event to Putside Task or calls Putside callback function or Functor to WakeUp Putside.
			}
		}
	}

	//Universal call R/W interface wrappers with blocking for ptrSignal_t
	template <typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Write_bl(DataQueue_t<ptrSignal_t,ptrGetsideFunctorT,SIZE,DataT>* ptrDataQueue, DataT* ptrDataBuf, uint8_t Size)
	{
		uint8_t SizeWritten = 0;
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while(SizeWritten < Size) if( !ptrDataQueue->Write(ptrDataBuf, Size, SizeWritten) ) Wait();
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	template <typename ptrPutsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Read_bl(DataQueue_t<ptrPutsideFunctorT,ptrSignal_t,SIZE,DataT>* ptrDataQueue, DataT* ptrDataBuf, uint8_t Size)
	{
		uint8_t SizeRed = 0;
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while(SizeRed < Size) if( !ptrDataQueue->Read(ptrDataBuf, Size, SizeRed) ) Wait();
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	template <typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Put_bl(DataQueue_t<ptrSignal_t,ptrGetsideFunctorT,SIZE,DataT>* ptrDataQueue, const DataT& rData)
	{
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while( !ptrDataQueue->Put(rData) ) Wait();
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	template <typename ptrPutsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Get_bl(DataQueue_t<ptrPutsideFunctorT,ptrSignal_t,SIZE,DataT>* ptrDataQueue, DataT& rData)
	{
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while( !ptrDataQueue->Get(rData) ) Wait();
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	
	//Universal call R/W interface wrappers with blocking for ptrEvent_t
	template <typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Write_bl(DataQueue_t<ptrEvent_t,ptrGetsideFunctorT,SIZE,DataT>* ptrDataQueue, DataT* ptrDataBuf, uint8_t Size)
	{
		uint8_t SizeWritten = 0;
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while(SizeWritten < Size) if( !ptrDataQueue->Write(ptrDataBuf, Size, SizeWritten) ) Wait();
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	template <typename ptrPutsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Read_bl(DataQueue_t<ptrPutsideFunctorT,ptrEvent_t,SIZE,DataT>* ptrDataQueue, DataT* ptrDataBuf, uint8_t Size)
	{
		uint8_t SizeRed = 0;
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while(SizeRed < Size) if( !ptrDataQueue->Read(ptrDataBuf, Size, SizeRed) ) Wait();
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	template <typename ptrGetsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Put_bl(DataQueue_t<ptrEvent_t,ptrGetsideFunctorT,SIZE,DataT>* ptrDataQueue, const DataT& rData)
	{
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while( !ptrDataQueue->Put(rData) ) Wait();
		(ptrDataQueue->GetPutsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}
	template <typename ptrPutsideFunctorT, uint8_t SIZE, typename DataT>
	inline void Get_bl(DataQueue_t<ptrPutsideFunctorT,ptrEvent_t,SIZE,DataT>* ptrDataQueue, DataT& rData)
	{
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Active);
		while( !ptrDataQueue->Get(rData) ) Wait();
		(ptrDataQueue->GetGetsidePtrFunctor())->SetMode(SignalMode_t::Silent);
	}

}
#endif