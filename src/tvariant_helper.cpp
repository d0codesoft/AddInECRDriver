#include "pch.h"
#include "tvariant_helper.h"
#include "str_utils.h"
#include <string>
#include <sstream>
#include <iomanip>

std::optional<std::wstring> VariantHelper::getStringValue(const tVariant& var) {
    switch (TV_VT(&var))
    {
    case VTYPE_PWSTR: {
        std::wstring wstr = str_utils::to_wstring(var.pwstrVal);
        return wstr;
    }
    case VTYPE_PSTR:
#ifdef _WIN32
        return str_utils::to_wstring(std::string(var.pstrVal), "windows-1251");
#else
        return str_utils::to_wstring(std::string(var.pstrVal));
#endif
    }
    return std::nullopt;
}

std::optional<long> VariantHelper::getLongValue(const tVariant& var) {
    switch (TV_VT(&var))
    {
    case VTYPE_HRESULT:
        return static_cast<long>(var.hRes);
    case VTYPE_I4:
        return static_cast<long>(TV_I4(&var));
    case VTYPE_I2:
        return static_cast<long>(TV_I2(&var));
    case VTYPE_I1:
        return static_cast<long>(TV_I1(&var));
    case VTYPE_INT:
        return static_cast<long>(TV_INT(&var));
    case VTYPE_UI4:
        return static_cast<long>(TV_UI4(&var));
    case VTYPE_UI2:
        return static_cast<long>(TV_UI2(&var));
    case VTYPE_UI1:
        return static_cast<long>(TV_UI1(&var));
    case VTYPE_UINT:
        return static_cast<long>(TV_UINT(&var));
    }
    return std::nullopt;
}

std::optional<bool> VariantHelper::getBoolValue(const tVariant& var) {
    if (TV_VT(&var) == VTYPE_BOOL) {
        return TV_BOOL(&var);
    }
    return std::nullopt;
}

std::optional<int> VariantHelper::getIntValue(const tVariant& var) {
    switch (TV_VT(&var))
    {
    case VTYPE_I4:
        return static_cast<int>(TV_I4(&var));
    case VTYPE_I2:
        return static_cast<int>(TV_I2(&var));
    case VTYPE_I1:
        return static_cast<int>(TV_I1(&var));
    case VTYPE_INT:
        return static_cast<int>(TV_INT(&var));
    case VTYPE_UI4:
        return static_cast<int>(TV_UI4(&var));
    case VTYPE_UI2:
        return static_cast<int>(TV_UI2(&var));
    case VTYPE_UI1:
        return static_cast<int>(TV_UI1(&var));
    case VTYPE_UINT:
        return static_cast<int>(TV_UINT(&var));
    case VTYPE_ERROR:
		return static_cast<int>(var.errCode);
    }
    return std::nullopt;
}

std::optional<DateTime> VariantHelper::getDateTimeValue(const tVariant& var)
{
	if (TV_VT(&var) == VTYPE_DATE) {
		return std::optional<DateTime>(
                std::chrono::system_clock::from_time_t(static_cast<time_t>(TV_DATE(&var)))
        );
	}
    return std::nullopt;
}

std::optional<double> VariantHelper::getDoubleValue(const tVariant& var)
{
    switch (TV_VT(&var))
    {
    case VTYPE_R4:
        return static_cast<double>(TV_R4(&var));
    case VTYPE_R8:
        return static_cast<double>(TV_R8(&var));
    case VTYPE_DATE:
        return static_cast<double>(TV_DATE(&var));
	case VTYPE_I4:
		return static_cast<double>(TV_I4(&var));
	case VTYPE_I2:
		return static_cast<double>(TV_I2(&var));
	case VTYPE_I1:
		return static_cast<double>(TV_I1(&var));
	case VTYPE_INT:
		return static_cast<double>(TV_INT(&var));
	case VTYPE_UI4:
		return static_cast<double>(TV_UI4(&var));
	case VTYPE_UI2:
		return static_cast<double>(TV_UI2(&var));
	case VTYPE_UI1:
		return static_cast<double>(TV_UI1(&var));
	case VTYPE_UINT:
		return static_cast<double>(TV_UINT(&var));
    }
    return std::nullopt;
}

std::optional<std::wstring> VariantHelper::getStrDoubleValue(const tVariant& var)
{
	if (VariantHelper::isValueDouble(var)) {
		auto amount = VariantHelper::getDoubleValue(var);
		if (amount.has_value()) {
			std::wostringstream oss;
			oss << std::fixed << std::setprecision(2) << amount.value();
			return oss.str();
		}
	}
	return std::nullopt;
}

bool VariantHelper::isValueString(const tVariant& var) {
    return TV_VT(&var) == VTYPE_PWSTR || TV_VT(&var) == VTYPE_PSTR;
}

bool VariantHelper::isValueInt(const tVariant& var) {
    return TV_VT(&var) == VTYPE_I4 || TV_VT(&var) == VTYPE_INT
        || TV_VT(&var) == VTYPE_I1 || TV_VT(&var) == VTYPE_UI4
        || TV_VT(&var) == VTYPE_UI2 || TV_VT(&var) == VTYPE_UI1
        || TV_VT(&var) == VTYPE_UINT || TV_VT(&var) == VTYPE_ERROR;
}

bool VariantHelper::isValueLong(const tVariant& var)
{
	return TV_VT(&var) == VTYPE_I4 || TV_VT(&var) == VTYPE_INT
		|| TV_VT(&var) == VTYPE_I1 || TV_VT(&var) == VTYPE_UI4
		|| TV_VT(&var) == VTYPE_UI2 || TV_VT(&var) == VTYPE_UI1
		|| TV_VT(&var) == VTYPE_UINT || TV_VT(&var) == VTYPE_ERROR
        || TV_VT(&var) == VTYPE_HRESULT;
}

bool VariantHelper::isValueBool(const tVariant& var) {
    return TV_VT(&var) == VTYPE_BOOL;
}

bool VariantHelper::isValueDouble(const tVariant& var) {
    return TV_VT(&var) == VTYPE_R8 || TV_VT(&var) == VTYPE_R4;
}

bool VariantHelper::isValueDate(const tVariant& var) {
    return TV_VT(&var) == VTYPE_DATE;
}

bool VariantHelper::isValueArray(const tVariant& var) {
    return TV_ISARRAY(&var);
}

bool VariantHelper::isValueEmpty(const tVariant& var) {
    return TV_VT(&var) == VTYPE_EMPTY;
}

std::wstring VariantHelper::getTypeValue(const tVariant& var)
{
	switch (TV_VT(&var))
	{
	case VTYPE_EMPTY: return L"EMPTY";
	case VTYPE_NULL: return L"NULL";
	case VTYPE_I2: return L"I2";
	case VTYPE_I4: return L"I4";
	case VTYPE_R4: return L"R4";
	case VTYPE_R8: return L"R8";
	case VTYPE_DATE: return L"DATE";
	case VTYPE_TM: return L"TM";
	case VTYPE_PSTR: return L"PSTR";
	case VTYPE_INTERFACE: return L"INTERFACE";
	case VTYPE_ERROR: return L"ERROR";
	case VTYPE_BOOL: return L"BOOL";
	case VTYPE_VARIANT: return L"VARIANT";
	case VTYPE_I1: return L"I1";
	case VTYPE_UI1: return L"UI1";
	case VTYPE_UI2: return L"UI2";
	case VTYPE_UI4: return L"UI4";
	case VTYPE_I8: return L"I8";
	case VTYPE_UI8: return L"UI8";
	case VTYPE_INT: return L"INT";
	case VTYPE_UINT: return L"UINT";
	case VTYPE_HRESULT: return L"HRESULT";
	case VTYPE_PWSTR: return L"PWSTR";
	case VTYPE_BLOB: return L"BLOB";
	case VTYPE_CLSID: return L"CLSID";
	case VTYPE_STR_BLOB: return L"STR_BLOB";
	case VTYPE_VECTOR: return L"VECTOR";
	case VTYPE_ARRAY: return L"ARRAY";
	case VTYPE_BYREF: return L"BYREF";
	case VTYPE_RESERVED: return L"RESERVED";
	case VTYPE_ILLEGAL: return L"ILLEGAL";
	default: return L"UNKNOWN";
	}
}
