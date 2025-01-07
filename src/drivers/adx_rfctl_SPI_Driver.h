//$Id: adx_rfctl_SPI_Driver.h 339 2025-01-02 23:45:33Z apolv $

#if !defined(DRIVERS_ADX_RFCTL_SPI_DRIVER_H_)
#define  DRIVERS_ADX_RFCTL_SPI_DRIVER_H_

#include "config/adx_drivers_config.h"

#include "core/adx_SystemStatus.h"

namespace adx_fsm
{
	enum class SPI_Speed_t: uint8_t
	{
		Slow = 0,
		Fast = 1
	};
	void MasterSPI_Config();
	void MasterSPI_Write(uint8_t PortNumber, uint8_t* ptrData, uint8_t DataSize, SPI_Speed_t Speed);
	void MasterSPI_Read(uint8_t PortNumber, uint8_t* ptrData, uint8_t DataSize, SPI_Speed_t Speed);
	void MasterSPI_SetErrorHandler(adx_fsm::ErrorHandler_t ErrorHandler);
}


#endif //DRIVERS_ADX_RFCTL_SPI_DRIVER_H_