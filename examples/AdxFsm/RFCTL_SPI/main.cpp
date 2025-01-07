//$Id: main.cpp 339 2025-01-02 23:45:33Z apolv $

#include "drivers/adx_rfctl_SPI_Driver.h"

using namespace adx_fsm;

int main(void)
{
	uint8_t DataBuf[] = {1,2,3,4,5};
	MasterSPI_Config();
	while (true)
	{
		MasterSPI_Read(3,DataBuf,5,SPI_Speed_t::Fast);
	}
}

