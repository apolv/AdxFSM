//$Id: adx_kernel_config.h 294 2024-11-28 16:35:06Z apolv $

/*Configuration for kernel/xxx source files*/

#if !defined(KERNEL_ADX_KERNEL_CONFIG_H_)
#define KERNEL_ADX_KERNEL_CONFIG_H_

#include <stdint.h>

namespace adx_kernel
{
	const uint16_t SystemMainStack_SIZE = 256U;
	uint8_t* const ptrSystemMainStackBottom = (uint8_t*)0x5fff;
}

#endif /* KERNEL_ADX_KERNEL_CONFIG_H_ */