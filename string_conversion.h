#pragma once

#ifndef STRING_CONVERSION_H
#define STRING_CONVERSION_H

#include <cstddef>
#include <cstdint>
#include <types.h>
#include <string>
#include "common_types.h"

//uint32_t convToShortWChar(WCHAR_T** Dest, const wchar_t* Source, size_t len = 0);
//uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
//uint32_t getLenShortWcharStr(const WCHAR_T* Source);
std::u16string utf8ToUtf16(const std::string& utf8);
std::string utf16ToUtf8(const std::u16string& utf16);
std::string wcharToString(const wchar_t* wstr);
std::string convFromShortToString(const WCHAR_T* Source, uint32_t len);
std::string convertDriverDescriptionToJson(const DriverDescription& desc);
//char* convertWCharToChar(const WCHAR_T* wstr);

std::string LoadStringResource(UINT resourceId);

#endif // STRING_CONVERSION_H
