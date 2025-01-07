//$Id: adx_UsartDriver.cpp 274 2024-11-15 11:35:28Z apolv $

#include "drivers/adx_UsartDriver.h"

#include <avr/pgmspace.h>

using namespace adx_fsm;

ErrorHandler_t UsartDriver_t::ErrorHandler{ nullptr };

bool UsartDriver_t::Config(const Config_t* ptrConfigFromFlash)
{
	{	//Enter to critical section: interrupts are off
		CriticalSection_t cs;
		if( !IsTxComplete() ) {return false;}
		//Set up Port Rx/Tx Pins according to ptrUsartIOBase
		switch(uint16_t(ptrUsartIOBase))
		{
		case uint16_t(&(USARTC0)):
			PORTC.OUTSET = PIN3_bm; //Sets Tx pin high
			PORTC.DIRSET = PIN3_bm; //Sets Tx pin as output
			PORTC.DIRCLR = PIN2_bm; //Sets Rx pin as input
			break;
		case uint16_t(&(USARTC1)):
			PORTC.OUTSET = PIN7_bm;
			PORTC.DIRSET = PIN7_bm;
			PORTC.DIRCLR = PIN6_bm;
			break;
		case uint16_t(&(USARTD0)):
			PORTD.OUTSET = PIN3_bm;
			PORTD.DIRSET = PIN3_bm;
			PORTD.DIRCLR = PIN2_bm;
			break;
		case uint16_t(&(USARTD1)):
			PORTD.OUTSET = PIN7_bm;
			PORTD.DIRSET = PIN7_bm;
			PORTD.DIRCLR = PIN6_bm;
			break;
		case uint16_t(&(USARTE0)):
			PORTE.OUTSET = PIN3_bm;
			PORTE.DIRSET = PIN3_bm;
			PORTE.DIRCLR = PIN2_bm;
			break;
		case uint16_t(&(USARTE1)):
			PORTE.OUTSET = PIN7_bm;
			PORTE.DIRSET = PIN7_bm;
			PORTE.DIRCLR = PIN6_bm;
			break;
		default:;
		}
		uint8_t tmpUsartCTRLA =  pgm_read_byte(&(ptrConfigFromFlash->CTRLA));
		ptrUsartIOBase->CTRLA = tmpUsartCTRLA & 0b1111'1100; //setting up USART interrupts except DREINTLVL[1:0]
		DRE_InterruptLevel = pgm_read_byte(&(ptrConfigFromFlash->CTRLA)) & 0b0000'0011; //Store DREINTLVL[1:0]
		ptrUsartIOBase->BAUDCTRLA = pgm_read_byte(&(ptrConfigFromFlash->BAUDCTRLA)); //setting up baud rate
		ptrUsartIOBase->BAUDCTRLB = pgm_read_byte(&(ptrConfigFromFlash->BAUDCTRLB)); //setting up baud rate
		ptrUsartIOBase->CTRLC = pgm_read_byte(&(ptrConfigFromFlash->CTRLC)); //setting up USART Mode and Frame format
		ptrUsartIOBase->CTRLB = pgm_read_byte(&(ptrConfigFromFlash->CTRLB)); //enables USART Rx and Tx
		
		//Turns on USART PMIC Interrupts based on settings
		switch (UsartSettings_t::TxInterruptPriority(tmpUsartCTRLA & 0b0000'0011))
		{
		case UsartSettings_t::TxInterruptPriority::Low:
			PMIC.CTRL |= PMIC_LOLVLEN_bm;
			break;
		case UsartSettings_t::TxInterruptPriority::Middle:
			PMIC.CTRL |= PMIC_MEDLVLEX_bm;
			break;
		case UsartSettings_t::TxInterruptPriority::High:
			PMIC.CTRL |= PMIC_HILVLEN_bm;
		default:;
		}
		switch (UsartSettings_t::RxInterruptPriority(tmpUsartCTRLA & 0b0011'0000))
		{
		case UsartSettings_t::RxInterruptPriority::Low:
			PMIC.CTRL |= PMIC_LOLVLEN_bm;
			break;
		case UsartSettings_t::RxInterruptPriority::Middle:
			PMIC.CTRL |= PMIC_MEDLVLEX_bm;
			break;
		case UsartSettings_t::RxInterruptPriority::High:
			PMIC.CTRL |= PMIC_HILVLEN_bm;
		default:;
		}
		
	} //Exit from critical section
	return true;
}

UsartRxDataQueue_t* UsartDriver_t::OpenRead(ptrEvent_t ptrEvent)
{
	if(ReadAccess_mx.TryLock(ptrEvent))
	{
		RxDataBuffer.ConnectGetsideFunctor(ptrEvent, false);
		return &RxDataBuffer;
	}
	return nullptr;
}

UsartRxDataQueue_t* UsartDriver_t::OpenRead_bl(ptrEvent_t ptrEvent)
{
	ReadAccess_mx.Lock_bl(ptrEvent);
	RxDataBuffer.ConnectGetsideFunctor(ptrEvent, false);
	return &RxDataBuffer;
}

void UsartDriver_t::CloseRead()
{
	CriticalSection_t cs;
	ptrEvent_t ptrPrevEvent = ptrEvent_t(RxDataBuffer.GetGetsidePtrFunctor());
	if(ptrPrevEvent != nullptr)
	{
		SignalMode_t PrevMode = ptrPrevEvent->GetMode();
		ptrPrevEvent->SetMode(SignalMode_t::Disabled);
		RxDataBuffer.ConnectGetsideFunctor(nullptr, false);
		ptrPrevEvent->SetMode(PrevMode);
	}
	else RxDataBuffer.ConnectGetsideFunctor(nullptr, false);
	ReadAccess_mx.Unlock();
}

UsartTxDataQueue_t* UsartDriver_t::OpenWrite(ptrEvent_t ptrEvent)
{
	if(WriteAccess_mx.TryLock(ptrEvent))
	{
		TxDataBuffer.ConnectPutsideFunctor(ptrEvent, false);
		return &TxDataBuffer;
	}
	return nullptr;
}

UsartTxDataQueue_t* UsartDriver_t::OpenWrite_bl(ptrEvent_t ptrEvent)
{
	WriteAccess_mx.Lock_bl(ptrEvent);
	TxDataBuffer.ConnectPutsideFunctor(ptrEvent, false);
	return &TxDataBuffer;
}

void UsartDriver_t::CloseWrite()
{
	CriticalSection_t cs;
	ptrEvent_t ptrPrevEvent = ptrEvent_t(TxDataBuffer.GetPutsidePtrFunctor());
	if(ptrPrevEvent != nullptr)
	{
		SignalMode_t PrevMode = ptrPrevEvent->GetMode();
		ptrPrevEvent->SetMode(SignalMode_t::Disabled);
		TxDataBuffer.ConnectPutsideFunctor(nullptr, false);
		ptrPrevEvent->SetMode(PrevMode);
	}
	else TxDataBuffer.ConnectPutsideFunctor(nullptr, false);
	WriteAccess_mx.Unlock();
	
}
