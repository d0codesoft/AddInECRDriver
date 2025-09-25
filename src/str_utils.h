#pragma once

#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>
#include <iostream>

namespace str_utils
{

	std::wstring to_wstring(const std::string& str, const std::string& encoding = "UTF-8");
	std::wstring to_wstring(const std::u16string& str);
	std::wstring to_wstring(const char* str, const std::string& encoding = "UTF-8");
	std::wstring to_wstring(const char16_t* str);

	std::u16string to_u16string(const std::string& str, const std::string& encoding = "UTF-8");
	std::u16string to_u16string(const std::wstring& str);
	std::u16string to_u16string(const char* str, const std::string& encoding = "UTF-8");
	std::u16string to_u16string(const wchar_t* str);

	std::string to_string(const std::wstring& str);
	std::string to_string(const std::u16string& str);
	std::string to_string(const wchar_t* str);
	std::string to_string(const char16_t* str);

} // namespace str_utils

#endif // STR_UTILS_H

