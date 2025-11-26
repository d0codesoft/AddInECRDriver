// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "logger.h"

#ifdef _WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        auto logDir = Logger::getLogDriverFilePath();
        LOGGER_INITIALIZE_DEFAULT(logDir);
        LOG_INFO_ADD(L"Driver", L"Driver loaded");
        break;
    }
    case DLL_PROCESS_DETACH:
        LOG_INFO_ADD(L"Driver", L"Driver unloaded");
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

#else // POSIX (Linux/macOS)

__attribute__((constructor))
static void lib_init()
{
    auto logDir = Logger::getLogDriverFilePath();
    LOGGER_INITIALIZE_DEFAULT(logDir);
    LOG_INFO_ADD(L"Driver", L"Driver loaded");
}

__attribute__((destructor))
static void lib_cleanup()
{
    LOG_INFO_ADD(L"Driver", L"Driver unloaded");
}

#endif