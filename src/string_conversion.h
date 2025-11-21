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
std::u16string LoadStringResourceFor1C(const std::wstring_view& resourceId);
std::wstring LoadStringResource(const std::wstring_view& resourceId);
std::u16string convertDriverDescriptionToJson(const DriverDescription& desc);

#endif // STRING_CONVERSION_H
