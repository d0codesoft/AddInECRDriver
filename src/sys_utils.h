#pragma once

#ifndef SYSUTILS_H
#define SYSUTILS_H

#include <string>

class SysUtils
{
public: 
	SysUtils() = delete;

	static std::wstring get_full_path(const std::wstring& path);
	static std::wstring getLogDriverFilePath();
	static HMODULE g_hModule;
	
	static void initLogging();
	static void LogInfo(const std::wstring& message);
	static void LogWarning(const std::wstring& message);
	static void LogError(const std::wstring& message);
	static void LogException(const std::wstring& message, const std::exception& ex);

};

#endif // SYSUTILS_H

