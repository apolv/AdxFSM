//$Id: adx_rfctl_SPI_Driver.cpp 339 2025-01-02 23:45:33Z apolv $

#include "drivers/adx_rfctl_SPI_Driver.h"

#include <avr/io.h>
#include "core/adx_SystemQueue.h"

using namespace adx_fsm;

static const uint8_t SPI_Ctrl_FastSpeed = 0b1101'0000;//Fsck=Fper/2
static const uint8_t SPI_Ctrl_LowSpeed = 0b1101'0010;//Fsck=Fper/32
static const uint8_t USART_Baudctrla_FastSpeed = 0;//Fsck=Fper/2
static const uint8_t USART_Baudctrla_LowSpeed = 15;//Fsck=Fper/32

static ErrorHandler_t SPI_ErrorHandler{ nullptr };
static const uint8_t Error_AttemptToAccessToUndefinedSPIport = 1; //Attempt to access to undefined SPI port Number.
static const uint16_t FileNameHash = CRC16_Modbus("adx_rfctl_SPI_Driver.cpp");

void adx_fsm::MasterSPI_Config()
{
	#if (RFCTL_SPI_PORT_MASK & (0b1000'0001))
		//SPIE Configuration:
		#if (RFCTL_SPI_PORT_MASK & (0x1<<0))
			//CS_0 pin Config:
			PORTE.DIRSET = (0x1<<4);
			PORTE.OUTSET = (0x1<<4);
		#endif
		#if (RFCTL_SPI_PORT_MASK & (0x1<<7))
			//CS_7 pin Config:
			RFCTL_SPI7_SS_PORT.DIRSET = (0x1<<RFCTL_SPI7_SS_PIN_POSITION);
			RFCTL_SPI7_SS_PORT.OUTSET = (0x1<<RFCTL_SPI7_SS_PIN_POSITION);
		#endif
		PORTE.DIRSET = 0b1010'0000;
		SPIE.CTRL = SPI_Ctrl_FastSpeed;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0b0100'0010))
		//SPID Configuration:
		#if (RFCTL_SPI_PORT_MASK & (0x1<<1))
			//CS_1 pin Config:
			PORTD.DIRSET = (0x1<<4);
			PORTD.OUTSET = (0x1<<4);
		#endif
		#if (RFCTL_SPI_PORT_MASK & (0x1<<6))
			//CS_6 pin Config:
			RFCTL_SPI6_SS_PORT.DIRSET = (0x1<<RFCTL_SPI6_SS_PIN_POSITION);
			RFCTL_SPI6_SS_PORT.OUTSET = (0x1<<RFCTL_SPI6_SS_PIN_POSITION);
		#endif
		PORTD.DIRSET = 0b1010'0000;
		SPID.CTRL = SPI_Ctrl_FastSpeed;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0b0010'0100))
		//SPIC Configuration:
		#if (RFCTL_SPI_PORT_MASK & (0x1<<2))
			//CS_2 pin Config:
			PORTC.DIRSET = (0x1<<4);
			PORTC.OUTSET = (0x1<<4);
		#endif
		#if (RFCTL_SPI_PORT_MASK & (0x1<<5))
			//CS_5 pin Config:
			RFCTL_SPI5_SS_PORT.DIRSET = (0x1<<RFCTL_SPI5_SS_PIN_POSITION);
			RFCTL_SPI5_SS_PORT.OUTSET = (0x1<<RFCTL_SPI5_SS_PIN_POSITION);
		#endif
		PORTC.DIRSET = 0b1010'0000;
		SPIC.CTRL = SPI_Ctrl_FastSpeed;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0b0001'1000))
		//USART C0 SPI Configuration:
		#if (RFCTL_SPI_PORT_MASK & (0x1<<3))
			//CS_3 pin Config:
			RFCTL_SPI3_SS_PORT.DIRSET = (0x1<<RFCTL_SPI3_SS_PIN_POSITION);
			RFCTL_SPI3_SS_PORT.OUTSET = (0x1<<RFCTL_SPI3_SS_PIN_POSITION);
		#endif
		#if (RFCTL_SPI_PORT_MASK & (0x1<<4))
			//CS_4 pin Config:
			RFCTL_SPI4_SS_PORT.DIRSET = (0x1<<RFCTL_SPI4_SS_PIN_POSITION);
			RFCTL_SPI4_SS_PORT.OUTSET = (0x1<<RFCTL_SPI4_SS_PIN_POSITION);
		#endif
		PORTC.DIRSET = 0b0000'1010;//set MOSI and XCK pins to output
		PORTC.DIRCLR = 0b0000'0100;//set MISO pin to input
		USARTC0.BAUDCTRLA = USART_Baudctrla_FastSpeed;
		USARTC0.BAUDCTRLB = 0;
		USARTC0.CTRLC = 0b1100'0000;
		USARTC0.CTRLB = 0b0001'1000;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x300))
		//USART F0 SPI Configuration:
		#if (RFCTL_SPI_PORT_MASK & (0x1<<8))
			//CS_8 pin Config:
			RFCTL_SPI8_SS_PORT.DIRSET = (0x1<<RFCTL_SPI8_SS_PIN_POSITION);
			RFCTL_SPI8_SS_PORT.OUTSET = (0x1<<RFCTL_SPI8_SS_PIN_POSITION);
		#endif
		#if (RFCTL_SPI_PORT_MASK & (0x1<<9))
			//CS_9 pin Config:
			RFCTL_SPI9_SS_PORT.DIRSET = (0x1<<RFCTL_SPI9_SS_PIN_POSITION);
			RFCTL_SPI9_SS_PORT.OUTSET = (0x1<<RFCTL_SPI9_SS_PIN_POSITION);
		#endif
		PORTF.DIRSET = 0b0000'1010;//set MOSI and XCK pins to output
		PORTF.DIRCLR = 0b0000'0100;//set MISO pin to input
		USARTF0.BAUDCTRLA = USART_Baudctrla_FastSpeed;
		USARTF0.BAUDCTRLB = 0;
		USARTF0.CTRLC = 0b1100'0000;
		USARTF0.CTRLB = 0b0001'1000;
	#endif
}


void adx_fsm::MasterSPI_Write(uint8_t PortNumber, uint8_t* ptrData, uint8_t DataSize, SPI_Speed_t Speed)
{
	if(DataSize == 0) return;
	DataSize--;
	uint8_t Data = ptrData[DataSize];
	switch (PortNumber)
	{
	#if (RFCTL_SPI_PORT_MASK & (0x1<<0))
	case 0:
		SPIE.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		PORTE.OUTCLR = (0x1<<4);
		while(true) 
		{
			SPIE.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIE.STATUS & 0b1000'0000) == 0);
			}
			else
			{
				while ((SPIE.STATUS & 0b1000'0000) == 0);
				break;
			}
		}
		PORTE.OUTSET = (0x1<<4);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<1))
	case 1:
		SPID.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		PORTD.OUTCLR = (0x1<<4);
		while(true) 
		{
			SPID.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPID.STATUS & 0b1000'0000) == 0);
			}
			else
			{
				while ((SPID.STATUS & 0b1000'0000) == 0);
				break;
			}
		}
		PORTD.OUTSET = (0x1<<4);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<2))
	case 2:
		SPIC.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		PORTC.OUTCLR = (0x1<<4);
		while(true) 
		{
			SPIC.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIC.STATUS & 0b1000'0000) == 0);
			}
			else
			{
				while ((SPIC.STATUS & 0b1000'0000) == 0);
				break;
			}
		}
		PORTC.OUTSET = (0x1<<4);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<3))
	case 3:
		USARTC0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI3_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI3_SS_PIN_POSITION);
		while(true)
		{
			USARTC0.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTC0.STATUS & 0b0010'0000) == 0);
			}
			else
			{
				while ((USARTC0.STATUS & 0b0100'0000) == 0);
				USARTC0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI3_SS_PORT.OUTSET = (0x1<<RFCTL_SPI3_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<4))
	case 4:
		USARTC0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI4_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI4_SS_PIN_POSITION);
		while(true)
		{
			USARTC0.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTC0.STATUS & 0b0010'0000) == 0);
			}
			else
			{
				while ((USARTC0.STATUS & 0b0100'0000) == 0);
				USARTC0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI4_SS_PORT.OUTSET = (0x1<<RFCTL_SPI4_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<5))
	case 5:
		SPIC.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		RFCTL_SPI5_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI5_SS_PIN_POSITION);
		while(true)
		{
			SPIC.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIC.STATUS & 0b1000'0000) == 0);
			}
			else
			{
				while ((SPIC.STATUS & 0b1000'0000) == 0);
				break;
			}
		}
		RFCTL_SPI5_SS_PORT.OUTSET = (0x1<<RFCTL_SPI5_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<6))
	case 6:
		SPID.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		RFCTL_SPI6_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI6_SS_PIN_POSITION);
		while(true)
		{
			SPID.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPID.STATUS & 0b1000'0000) == 0);
			}
			else
			{
				while ((SPID.STATUS & 0b1000'0000) == 0);
				break;
			}
		}
		RFCTL_SPI6_SS_PORT.OUTSET = (0x1<<RFCTL_SPI6_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<7))
	case 7:
		SPIE.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		RFCTL_SPI7_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI7_SS_PIN_POSITION);
		while(true) 
		{
			SPIE.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIE.STATUS & 0b1000'0000) == 0);
			}
			else
			{
				while ((SPIE.STATUS & 0b1000'0000) == 0);
				break;
			}
		}
		RFCTL_SPI7_SS_PORT.OUTSET = (0x1<<RFCTL_SPI7_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<8))
	case 8:
		USARTF0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI8_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI8_SS_PIN_POSITION);
		while(true)
		{
			USARTF0.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTF0.STATUS & 0b0010'0000) == 0);
			}
			else
			{
				while ((USARTF0.STATUS & 0b0100'0000) == 0);
				USARTF0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI8_SS_PORT.OUTSET = (0x1<<RFCTL_SPI8_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<9))
	case 9:
		USARTF0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI9_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI9_SS_PIN_POSITION);
		while(true)
		{
			USARTF0.DATA = Data;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTF0.STATUS & 0b0010'0000) == 0);
			}
			else
			{
				while ((USARTF0.STATUS & 0b0100'0000) == 0);
				USARTF0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI9_SS_PORT.OUTSET = (0x1<<RFCTL_SPI9_SS_PIN_POSITION);
		return;
	#endif
	
	default:
		Error(Error_AttemptToAccessToUndefinedSPIport, static_cast<uint16_t>(__LINE__), FileNameHash, SPI_ErrorHandler);
		return;
	}
}
void adx_fsm::MasterSPI_Read(uint8_t PortNumber, uint8_t* ptrData, uint8_t DataSize, SPI_Speed_t Speed)
{
	if(DataSize == 0) return;
	DataSize--;
	uint8_t Data = ptrData[DataSize];
	uint8_t ReadIndex;
	switch (PortNumber)
	{
	#if (RFCTL_SPI_PORT_MASK & (0x1<<0))
	case 0:
		SPIE.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		PORTE.OUTCLR = (0x1<<4);
		while(true)
		{
			SPIE.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIE.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIE.DATA;
			}
			else
			{
				while ((SPIE.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIE.DATA;
				break;
			}
		}
		PORTE.OUTSET = (0x1<<4);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<1))
	case 1:
		SPID.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		PORTD.OUTCLR = (0x1<<4);
		while(true)
		{
			SPID.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPID.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPID.DATA;
			}
			else
			{
				while ((SPID.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPID.DATA;
				break;
			}
		}
		PORTD.OUTSET = (0x1<<4);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<2))
	case 2:
		SPIC.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		PORTC.OUTCLR = (0x1<<4);
		while(true)
		{
			SPIC.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIC.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIC.DATA;
			}
			else
			{
				while ((SPIC.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIC.DATA;
				break;
			}
		}
		PORTC.OUTSET = (0x1<<4);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<3))
	case 3:
		USARTC0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI3_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI3_SS_PIN_POSITION);
		while(true)
		{
			USARTC0.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTC0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTC0.DATA;
				USARTC0.STATUS = 0b1100'0000;
			}
			else
			{
				while ((USARTC0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTC0.DATA;
				USARTC0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI3_SS_PORT.OUTSET = (0x1<<RFCTL_SPI3_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<4))
	case 4:
		USARTC0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI4_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI4_SS_PIN_POSITION);
		while(true)
		{
			USARTC0.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTC0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTC0.DATA;
				USARTC0.STATUS = 0b1100'0000;
			}
			else
			{
				while ((USARTC0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTC0.DATA;
				USARTC0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI4_SS_PORT.OUTSET = (0x1<<RFCTL_SPI4_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<5))
	case 5:
		SPIC.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		RFCTL_SPI5_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI5_SS_PIN_POSITION);
		while(true)
		{
			SPIC.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIC.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIC.DATA;
			}
			else
			{
				while ((SPIC.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIC.DATA;
				break;
			}
		}
		RFCTL_SPI5_SS_PORT.OUTSET = (0x1<<RFCTL_SPI5_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<6))
	case 6:
		SPID.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		RFCTL_SPI6_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI6_SS_PIN_POSITION);
		while(true)
		{
			SPID.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPID.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPID.DATA;
			}
			else
			{
				while ((SPID.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPID.DATA;
				break;
			}
		}
		RFCTL_SPI6_SS_PORT.OUTSET = (0x1<<RFCTL_SPI6_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<7))
	case 7:
		SPIE.CTRL = (Speed == SPI_Speed_t::Fast) ? SPI_Ctrl_FastSpeed : SPI_Ctrl_LowSpeed;
		RFCTL_SPI7_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI7_SS_PIN_POSITION);
		while(true)
		{
			SPIE.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((SPIE.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIE.DATA;
			}
			else
			{
				while ((SPIE.STATUS & 0b1000'0000) == 0);
				ptrData[ReadIndex] = SPIE.DATA;
				break;
			}
		}
		RFCTL_SPI7_SS_PORT.OUTSET = (0x1<<RFCTL_SPI7_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<8))
	case 8:
		USARTF0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI8_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI8_SS_PIN_POSITION);
		while(true)
		{
			USARTF0.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTF0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTF0.DATA;
				USARTF0.STATUS = 0b1100'0000;
			}
			else
			{
				while ((USARTF0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTF0.DATA;
				USARTF0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI8_SS_PORT.OUTSET = (0x1<<RFCTL_SPI8_SS_PIN_POSITION);
		return;
	#endif
	
	#if (RFCTL_SPI_PORT_MASK & (0x1<<9))
	case 9:
		USARTF0.BAUDCTRLA = (Speed == SPI_Speed_t::Fast) ? USART_Baudctrla_FastSpeed : USART_Baudctrla_LowSpeed;
		RFCTL_SPI9_SS_PORT.OUTCLR = (0x1<<RFCTL_SPI9_SS_PIN_POSITION);
		while(true)
		{
			USARTF0.DATA = Data;
			ReadIndex = DataSize;
			if(DataSize != 0)
			{
				DataSize--;
				Data = ptrData[DataSize];
				while ((USARTF0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTF0.DATA;
				USARTF0.STATUS = 0b1100'0000;
			}
			else
			{
				while ((USARTF0.STATUS & 0b0100'0000) == 0);
				ptrData[ReadIndex] = USARTF0.DATA;
				USARTF0.STATUS = 0b1100'0000;
				break;
			}
		}
		RFCTL_SPI9_SS_PORT.OUTSET = (0x1<<RFCTL_SPI9_SS_PIN_POSITION);
		return;
	#endif
	
	default:
		Error(Error_AttemptToAccessToUndefinedSPIport, static_cast<uint16_t>(__LINE__), FileNameHash, SPI_ErrorHandler);
		return;
	}
}

void adx_fsm::MasterSPI_SetErrorHandler(ErrorHandler_t ErrorHandler) { SPI_ErrorHandler = ErrorHandler; }