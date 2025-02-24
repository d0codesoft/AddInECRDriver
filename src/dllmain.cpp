// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "sys_utils.h"
#include "logger.h"

#if !defined(OS_LINUX) && !defined(OS_MACOS)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
		auto logDir = SysUtils::getLogDriverFilePath();
		LOGGER_INITIALIZE_DEFAULT(logDir);
		LOG_INFO_ADD(L"Driver", L"Driver loaded");
        SysUtils::g_hModule = hModule;
        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
		LOG_INFO_ADD(L"Driver", L"Driver unloaded");
        break;
    }
    return TRUE;
}
#else
__attribute__((constructor)) void lib_init() {
    //SysUtils::initLogging(); // Initialize logging
}

__attribute__((destructor)) void lib_cleanup() {
}
#endif