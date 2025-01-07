//$Id: adx_UsartDriver.h 274 2024-11-15 11:35:28Z apolv $

#if !defined(DRIVERS_ADX_USART_DRIVER_H_)
#define DRIVERS_ADX_USART_DRIVER_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "core/adx_Event.h"
#include "core/adx_DataQueue.h"
#include "core/adx_RecursiveMutex.h"
#include "config/adx_drivers_config.h"

namespace adx_fsm
{
 	//Error and Warning Codes:
	constexpr uint8_t Warning_UsartRxDataBufferOverflow = 1; //At least one USART Rx Data Buffer is Full and Rx Data is lost
	//bit[2] = Rx Parity Error
	//bit[3] = Rx Hardware Data Register Buffer Overflow
	//bit[4] = Rx Frame Error (i.e. wrong baud rate or bad signal integrity)
	const uint16_t UsartDriverH_FileNameHash = CRC16_Modbus("adx_UsartDriver.h");
	
	class UsartDriver_t;
	using UsartTxDataQueue_t = DataQueue_t<ptrEvent_t,UsartDriver_t*,UsartDriverTxDataBuffer_SIZE,uint8_t>;
	using UsartRxDataQueue_t = DataQueue_t<bool (*)(),ptrEvent_t,UsartDriverRxDataBuffer_SIZE,uint8_t>;
	
	class UsartDriver_t
	{
		//friend UsartTxDataQueue_t;
	public:
		struct Config_t
		{
			uint8_t CTRLA;
			uint8_t CTRLB;
			uint8_t CTRLC;
			uint8_t BAUDCTRLA;
			uint8_t BAUDCTRLB;
		};
		UsartDriver_t() = delete;
		UsartDriver_t(USART_t* UsartBaseAddress) : ptrUsartIOBase(UsartBaseAddress) {}
		bool Config(const Config_t* ptrConfigFromFlash);
		UsartRxDataQueue_t* OpenRead(ptrEvent_t);
		UsartRxDataQueue_t* OpenRead_bl(ptrEvent_t);
		void CloseRead();
		UsartTxDataQueue_t* OpenWrite(ptrEvent_t);
		UsartTxDataQueue_t* OpenWrite_bl(ptrEvent_t);
		void CloseWrite();
		inline void PutDataToRxBufferFromISR(uint8_t);
		inline bool operator()();
		inline bool IsTxComplete() const {return TxCompleteFlag;}
		static void SetErrorHandler(ErrorHandler_t InErrorHandler) { ErrorHandler = InErrorHandler; }
	private:
		USART_t* ptrUsartIOBase{ nullptr };
		UsartTxDataQueue_t TxDataBuffer{nullptr,this,false,true};
		UsartRxDataQueue_t RxDataBuffer{nullptr,nullptr,false,false};
		RecursiveMutex_t ReadAccess_mx{};
		RecursiveMutex_t WriteAccess_mx{};
		uint8_t DRE_InterruptLevel{0};
		volatile bool TxCompleteFlag{ true };
		static ErrorHandler_t ErrorHandler;
	};
	using UsartConfig_t = UsartDriver_t::Config_t;
	
	//Implementation
	inline bool UsartDriver_t::operator()() //is invoked from ISR and from task by fifo.Put() call
	{
		uint8_t Data;
		CriticalSection_t cs;
		if(TxDataBuffer.Get(Data))
		{
			TxCompleteFlag = false;
			ptrUsartIOBase->DATA = Data;
			ptrUsartIOBase->CTRLA |= DRE_InterruptLevel;
		}
		else
		{
			ptrUsartIOBase->CTRLA &= 0b1111'1100;
			TxCompleteFlag = true;
		}
		return true;
	}
	inline void UsartDriver_t::PutDataToRxBufferFromISR(uint8_t Data)
	{
		uint8_t Warning_UsartRxStatus;
		Warning_UsartRxStatus = ptrUsartIOBase->STATUS & 0b0001'1100;
		if(Warning_UsartRxStatus != 0) Warning(Warning_UsartRxStatus, static_cast<uint16_t>(__LINE__), UsartDriverH_FileNameHash, UsartDriver_t::ErrorHandler);
		if(!RxDataBuffer.Put(Data)) Warning(Warning_UsartRxDataBufferOverflow, static_cast<uint16_t>(__LINE__), UsartDriverH_FileNameHash, UsartDriver_t::ErrorHandler);
	}

	//Creates compile-time constant config structure
	union UsartSettings_t
	{
		enum class FrameLen: uint8_t
		{
			b5 = 0,
			b6 = 0x01,
			b7 = 0x02,
			b8 = 0x03
		};
		enum class Parity: uint8_t
		{
			None = 0,
			Even = 0x02<<4,
			Odd = 0x03<<4
		};
		enum class StopBits: uint8_t
		{
			One = 0,
			Two = 0x01<<3
		};
		enum class RxInterruptPriority: uint8_t
		{
			Low = 0x01<<4,
			Middle = 0x02<<4,
			High = 0x03<<4
		};
		enum class TxInterruptPriority: uint8_t
		{
			Low = 0x01<<0,
			Middle = 0x02<<0,
			High = 0x03<<0
		};
	};
	constexpr UsartConfig_t GetConfig(
		const int64_t Fperipheral_Hz,
		const int64_t BaudRate_bps,
		const UsartSettings_t::FrameLen FrameLen,
		const UsartSettings_t::Parity Parity,
		const UsartSettings_t::StopBits StopBits,
		const UsartSettings_t::RxInterruptPriority RxIntPrty,
		const UsartSettings_t::TxInterruptPriority TxIntPrty)
	{
		UsartConfig_t cfg = { 0,0,0,0,0 };
		cfg.CTRLA |= uint8_t(RxIntPrty);
		cfg.CTRLA |= uint8_t(TxIntPrty);
		cfg.CTRLB = 0b00011000; //Rx Enable, Tx Enable, CLK2X = 0, Multiprocessor Communication Mode Disabled
		cfg.CTRLC |= uint8_t(FrameLen);
		cfg.CTRLC |= uint8_t(Parity);
		cfg.CTRLC |= uint8_t(StopBits);
		
		//Baud rate settings calculation
		int64_t Fper = Fperipheral_Hz;
		int64_t Fbaud = BaudRate_bps;
		int64_t delta_fbaud = 0;
		int64_t fbaud_real = 0;
		int64_t Bsel = 0; 
		int64_t Bsel_result = 4095;
		int64_t Bscale_result = 0;
		//int64_t Fbaud_result = 0;
		
		for(int64_t Bscale = -7; Bscale <= 7; Bscale++)
		{
			
			if(Bscale >= 0)
			{
				Bsel = (Fper - (1<<Bscale)*16*Fbaud)/((1<<Bscale)*16*Fbaud);
			}
			else
			{
				Bsel = ((1<<(-Bscale))*(Fper - 16*Fbaud))/(16*Fbaud);
			}
			for(int64_t i = 0; i<=1; i++)
			{
				if(Bscale >= 0)
				{
					fbaud_real = Fper/( (1<<Bscale) * 16 * (Bsel + i + 1) );
				}
				else
				{
					fbaud_real = ((1<<(-Bscale))*Fper)/(16*(Bsel + i + (1<<(-Bscale))));
				}
				delta_fbaud = fbaud_real - Fbaud;
				if(delta_fbaud < 0) delta_fbaud = -delta_fbaud;
				if(((delta_fbaud*1000)/Fbaud < 5) && ((Bsel + i) < Bsel_result) && ((Bsel + i) >= 0))
				{
					//Fbaud_result = fbaud_real;
					Bsel_result = Bsel + i;
					Bscale_result = Bscale;
				}
			}
		}
		if(Bsel_result == 0 && Bscale_result != 0)
		{
			if(Bscale_result >= 0)
			Bsel_result = (1<<Bscale_result) - 1;
			else
			Bsel_result = 0;
			Bscale_result = 0;
		}
		cfg.BAUDCTRLA = uint8_t(Bsel_result & 0x00'00'00'00'00'00'00'ff);
		uint8_t tmp = 0;
		tmp = uint8_t(Bscale_result & 0x00'00'00'00'00'00'00'0f);
		tmp = tmp << 4;
		tmp = tmp | uint8_t( (Bsel_result >> 8) & 0x00'00'00'00'00'00'00'0f);
		cfg.BAUDCTRLB = tmp;
		return cfg;
	}
}

#define ADX_USART(ADX_USART_NAME,AVR_USART_NAME) UsartDriver_t ADX_USART_NAME(&AVR_USART_NAME);\
ISR(AVR_USART_NAME##_DRE_vect) \
{ \
	ADX_USART_NAME(); \
}\
ISR(AVR_USART_NAME##_RXC_vect) \
{ \
	uint8_t Data; \
	Data = AVR_USART_NAME.DATA; \
	ADX_USART_NAME.PutDataToRxBufferFromISR(Data); \
}

#endif