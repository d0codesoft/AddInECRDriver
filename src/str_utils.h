#pragma once

#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>
#include <iostream>
#include <optional>

namespace str_utils
{

	std::wstring to_wstring(const std::string& str, const std::string& encoding = "UTF-8");
	std::wstring to_wstring(const std::u16string& str);
	std::wstring to_wstring(const char* str, const std::string& encoding = "UTF-8");
	std::wstring to_wstring(const char16_t* str);
	std::wstring to_wstring(const int value);
	std::wstring to_wstring(const size_t value);
	std::wstring to_wstring(const long value);

	std::u16string to_u16string(const std::string& str, const std::string& encoding = "UTF-8");
	std::u16string to_u16string(const std::wstring& str);
	std::u16string to_u16string(const char* str, const std::string& encoding = "UTF-8");
	std::u16string to_u16string(const wchar_t* str);
	std::u16string to_u16string(const char16_t* str);

	std::string to_string(const std::wstring& str);
	std::string to_string(const std::u16string& str);
	std::string to_string(const wchar_t* str);
	std::string to_string(const char16_t* str);

	bool iequals(const std::wstring& a, const std::wstring& b) noexcept;
	bool iequals(const std::string& a, const std::string& b) noexcept;
	std::optional<uint32_t> to_UInt(const std::wstring& value);
	std::optional<double> to_Double(const std::wstring& value);

} // namespace str_utils

#endif // STR_UTILS_H

