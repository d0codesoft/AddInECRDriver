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

inline void ltrim(std::string& s);
inline void rtrim(std::string& s);
inline void trim(std::string& s);
inline void ltrim(std::wstring& s);
inline void rtrim(std::wstring& s);
inline void trim(std::wstring& s);

}

#endif // STR_UTILS_H

