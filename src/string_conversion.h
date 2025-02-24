#pragma once

#include <cstddef>
#include <cstdint>
#include <types.h>
#include <string>
#include "common_types.h"

#ifndef STRING_CONVERSION_H
#define STRING_CONVERSION_H

std::u16string LoadStringResourceFor1C(const std::wstring& resourceId);
std::wstring LoadStringResource(const std::wstring& resourceId);
std::wstring u16stringToWstring(const std::u16string& u16str);
std::u16string wstringToU16string(const std::wstring& wstr);
std::u16string convertDriverDescriptionToJson(const DriverDescription& desc);
std::string convertWStringToString(const std::wstring& wstr);
std::wstring convertStringToWString(const std::string& str);

#endif // STRING_CONVERSION_H
