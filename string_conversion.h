#pragma once

#ifndef STRING_CONVERSION_H
#define STRING_CONVERSION_H

#include <cstddef>
#include <cstdint>
#include <types.h>
#include <string>
#include "common_types.h"

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
std::wstring convertDriverDescriptionToJson(const DriverDescription& desc);

#endif // STRING_CONVERSION_H
