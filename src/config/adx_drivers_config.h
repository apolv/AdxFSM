//$Id: adx_drivers_config.h 339 2025-01-02 23:45:33Z apolv $

#if !defined(CONFIG_ADX_DRIVERS_CONFIG_H_)
#define CONFIG_ADX_DRIVERS_CONFIG_H_

#include <stdint.h>

namespace adx_fsm
{
	////////// USART Driver settings //////////
	const uint8_t UsartDriverTxDataBuffer_SIZE = 16;
	const uint8_t UsartDriverRxDataBuffer_SIZE = 32;
	////////// End of USART Driver settings //////////
}

////////// RFCTL-2x SPI Settings /////////////
//RFCTL_SPI_PORT_MASK enables SPIx, where x - mask bit position
//SPI0 - SPIE, SS# port E, pin position #4 (starting 0)
//SPI7 - SPIE, SS# - user defined below
//SPI1 - SPID, SS# port D, pin position #4 (starting 0)
//SPI6 - SPID, SS# - user defined below
//SPI2 - SPIC, SS# port C, pin position #4 (starting 0)
//SPI5 - SPIC, SS# - user defined below
//SPI3 - USARTC0, SS# - user defined below
//SPI4 - USARTC0, SS# - user defined below
//SPI8 - USARTF0, SS# - user defined below
//SPI9 - USARTF0, SS# - user defined below
#include <avr/io.h>
#if !defined(RFCTL_SPI_PORT_MASK)
#define RFCTL_SPI_PORT_MASK 0x3ff
#endif

#define RFCTL_SPI3_SS_PIN_POSITION 3
#define RFCTL_SPI3_SS_PORT PORTB
#define RFCTL_SPI4_SS_PIN_POSITION 4
#define RFCTL_SPI4_SS_PORT PORTB
#define RFCTL_SPI5_SS_PIN_POSITION 5
#define RFCTL_SPI5_SS_PORT PORTB
#define RFCTL_SPI6_SS_PIN_POSITION 6
#define RFCTL_SPI6_SS_PORT PORTB
#define RFCTL_SPI7_SS_PIN_POSITION 7
#define RFCTL_SPI7_SS_PORT PORTB
#define RFCTL_SPI8_SS_PIN_POSITION 0
#define RFCTL_SPI8_SS_PORT PORTF
#define RFCTL_SPI9_SS_PIN_POSITION 4
#define RFCTL_SPI9_SS_PORT PORTF
////////// End of RFCTL-2x SPI Settings //////////

#endif