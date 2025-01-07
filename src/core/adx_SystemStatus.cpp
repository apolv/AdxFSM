//$Id: adx_SystemStatus.cpp 342 2025-01-05 00:40:58Z apolv $

#include "core/adx_SystemStatus.h"
#include "kernel/adx_kernel.h"

static adx_fsm::SystemStatus_t CurrentSystemStatus{ nullptr,0,0,0,adx_fsm::SystemStatus_t::Ok }; //Global status object with internal linkage
static void GetSystemStatus(adx_fsm::SystemStatus_t& rStatus) //local function with internal linkage
{
	adx_fsm::CriticalSection_t cs;
	rStatus = CurrentSystemStatus; //calls default bitwise copy constructor
	if (rStatus.Type == adx_fsm::SystemStatus_t::Warning)
	{
		CurrentSystemStatus.ErrorHandler = nullptr;
		CurrentSystemStatus.LineNumber = 0;
		CurrentSystemStatus.FileNameHash = 0;
		CurrentSystemStatus.ErrorCode = 0;
		CurrentSystemStatus.Type = adx_fsm::SystemStatus_t::Ok;
	}
	return;
}

void adx_fsm::Error(const uint8_t ErrorCode, const uint16_t LineNumber, const uint16_t FileNameHash, const ErrorHandler_t ErrorHandler) 
{
	adx_fsm::CriticalSection_t cs;
	if (CurrentSystemStatus.Type != adx_fsm::SystemStatus_t::Error)
	{
		CurrentSystemStatus.ErrorHandler = ErrorHandler;
		CurrentSystemStatus.LineNumber = LineNumber;
		CurrentSystemStatus.FileNameHash = FileNameHash;
		CurrentSystemStatus.ErrorCode = ErrorCode;
		CurrentSystemStatus.Type = adx_fsm::SystemStatus_t::Error;
	}
	return;
}
void adx_fsm::Warning(const uint8_t ErrorCode, const uint16_t LineNumber, const uint16_t FileNameHash, const ErrorHandler_t ErrorHandler)
{
	adx_fsm::CriticalSection_t cs;
	if (CurrentSystemStatus.Type == adx_fsm::SystemStatus_t::Ok)
	{
		CurrentSystemStatus.ErrorHandler = ErrorHandler;
		CurrentSystemStatus.LineNumber = LineNumber;
		CurrentSystemStatus.FileNameHash = FileNameHash;
		CurrentSystemStatus.ErrorCode = ErrorCode;
		CurrentSystemStatus.Type = adx_fsm::SystemStatus_t::Warning;
	}
	return;
}
void adx_fsm::Exit(const uint8_t ErrorCode, const uint16_t LineNumber, const uint16_t FileNameHash, const ErrorHandler_t ErrorHandler)
{
	adx_fsm::CriticalSection_t cs;
	if ( (CurrentSystemStatus.Type == adx_fsm::SystemStatus_t::Ok) || (CurrentSystemStatus.Type == adx_fsm::SystemStatus_t::Warning))
	{
		CurrentSystemStatus.ErrorHandler = ErrorHandler;
		CurrentSystemStatus.LineNumber = LineNumber;
		CurrentSystemStatus.FileNameHash = FileNameHash;
		CurrentSystemStatus.ErrorCode = ErrorCode;
		CurrentSystemStatus.Type = adx_fsm::SystemStatus_t::Exit;
	}
	return;
}
bool adx_fsm::ProcessSystemStatus(SystemStatus_t& rStatus)
{
	GetSystemStatus(rStatus);
	switch (rStatus.Type)
	{
	case SystemStatus_t::Ok:
		return true;
	case SystemStatus_t::Warning:
		if (rStatus.ErrorHandler != nullptr) 
		{
			rStatus.ErrorHandler(rStatus); 
		}
		return true;
	default: //Error or Exit
		if (rStatus.ErrorHandler != nullptr)
		{
			rStatus.ErrorHandler(rStatus);
		}
		return false;
	}
}