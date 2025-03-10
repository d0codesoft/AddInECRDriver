#pragma once

#ifndef COMMON_PLATFORM_H
#define COMMON_PLATFORM_H

#ifdef _WIN32
#include <wtypes.h>
#include <windows.h>
#define LIB_EXTENSION ".dll"
using LibHandle = HMODULE;
#define LoadSharedLibrary(libname) LoadLibraryW(libname)
#define GetFunctionAddress GetProcAddress
#define CloseSharedLibrary FreeLibrary
#else
#include <dlfcn.h>
#define LIB_EXTENSION ".so"
using LibHandle = void*;
#define LoadSharedLibrary(libname) dlopen(libname, RTLD_LAZY)
#define GetFunctionAddress dlsym
#define CloseSharedLibrary dlclose
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/unistr.h>
#include <unicode/ucsdet.h>
#include <unicode/utypes.h>
#include <stdexcept>
#include <vector>
#include <types.h>
#include "wide_console.h"
#include "str_utils.h"

void printConsole(const std::wstring& text);

std::wstring getLibraryName();

template<typename T>
T GetProcAddressSafe(HMODULE hModule, const char* procName) {
    T procAddress = reinterpret_cast<T>(GetFunctionAddress(hModule, procName));
    if (!procAddress) {
        std::wcout << L"Couldn't find the function: " << procName << std::endl;
    }
    return procAddress;
}

std::wstring getVariantValue(const tVariant& variant);

class SharedLibrary {
public:
    explicit SharedLibrary(const std::wstring& libName);
    ~SharedLibrary();

	bool isValid() const {
		return handle != nullptr;
	}

    LibHandle getHandle() const { return handle; }

private:
    LibHandle handle;
};

#endif //COMMON_PLATFORM_H