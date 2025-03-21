#pragma once

#ifndef TVARIANT_HELPER_H
#define TVARIANT_HELPER_H

#include <types.h>
#include <optional>
#include <string>
#include <chrono>

using DateTime = std::chrono::system_clock::time_point;

class VariantHelper
{
public:
    static std::optional<std::wstring> getStringValue(const tVariant& var);
    static std::optional<long> getLongValue(const tVariant& var);
    static std::optional<bool> getBoolValue(const tVariant& var);
    static std::optional<int> getIntValue(const tVariant& var);
    static std::optional<DateTime> getDateTimeValue(const tVariant& var);
    static std::optional<double> getDoubleValue(const tVariant& var);
    static bool isValueString(const tVariant& var);
    static bool isValueInt(const tVariant& var);
    static bool isValueLong(const tVariant& var);
    static bool isValueBool(const tVariant& var);
    static bool isValueDouble(const tVariant& var);
    static bool isValueDate(const tVariant& var);
    static bool isValueArray(const tVariant& var);
    static bool isValueEmpty(const tVariant& var);
};

#endif // TVARIANT_HELPER_H