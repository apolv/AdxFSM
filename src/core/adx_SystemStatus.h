//$Id: adx_SystemStatus.h 273 2024-11-15 08:53:09Z apolv $

#if !defined (CORE_ADX_SYSTEM_STATUS_H_)
#define CORE_ADX_SYSTEM_STATUS_H_

#include <stdint.h>

namespace adx_fsm
{
	struct SystemStatus_t;
	using ErrorHandler_t = void (*)(const SystemStatus_t&);
	struct SystemStatus_t
	{
		ErrorHandler_t ErrorHandler{ nullptr };
		uint16_t LineNumber{ 0 };
		uint16_t FileNameHash{ 0 };
		uint8_t ErrorCode{ 0 };
		enum : uint8_t
		{
			Ok,
			Warning,
			Exit,
			Error
		} Type{ Ok };
	};
	void Error(const uint8_t ErrorCode, const uint16_t LineNumber, const uint16_t FileNameHash, const ErrorHandler_t ErrorHandler = nullptr);
	void Warning(const uint8_t ErrorCode, const uint16_t LineNumber, const uint16_t FileNameHash, const ErrorHandler_t ErrorHandler = nullptr);
	void Exit(const uint8_t ErrorCode = 0, const uint16_t LineNumber = 0, const uint16_t FileNameHash = 0, const ErrorHandler_t ErrorHandler = nullptr);
	bool ProcessSystemStatus(SystemStatus_t& rStatus);

	constexpr uint16_t CRC16_Modbus(const char* str)
	{
		int i = 0;
		uint16_t reg = 0xffff;
		while (*(str + i) != 0)
		{
			reg ^= *(str + i);
			for (int j = 0; j < 8; j++)
			{
				if (reg & 0x01)
					reg = (reg >> 1) ^ 0xA001;
				else
					reg = reg >> 1;
			}
			i++;
		}
		return reg;
	}
}


#endif // !ADX_SYSTEM_STATUS_H_
