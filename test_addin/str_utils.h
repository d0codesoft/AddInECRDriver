#pragma once

#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>
#include <iostream>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ucsdet.h>
#include <unicode/utypes.h>
#include <unordered_map>

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

icu::UnicodeString to_UnicodeString(const std::string& str, const std::string& encoding = "UTF-8");
icu::UnicodeString to_UnicodeString(const std::wstring& str);
icu::UnicodeString to_UnicodeString(const std::u16string& str);
icu::UnicodeString to_UnicodeString(const char* str, const std::string& encoding = "UTF-8");
icu::UnicodeString to_UnicodeString(const wchar_t* str);
icu::UnicodeString to_UnicodeString(const char16_t* str);

std::wstring to_wstring(icu::UnicodeString& unicodeStr);
std::u16string to_u16string(icu::UnicodeString& unicodeStr);
std::string to_string(icu::UnicodeString& unicodeStr);

class strConverter
{
public:
	// Convert string to wstring using specified encoding
	static std::wstring to_wstring(const std::string& str, const std::string& encoding = "UTF-8");

	// Convert wstring to string using specified encoding
	static std::string to_string(const std::wstring& str, const std::string& toCoding = "UTF-8");

	// Destroy all converters
	static void destroy();
private:
	// Get converter for given encoding
	static UConverter* get_converter(const std::string& encoding);

	// Cache for converters
	static std::unordered_map<std::string, UConverter*> list_converter;
};

} // namespace str_utils

#endif // STR_UTILS_H

