#include "pch.h"
#include "string_conversion.h"
#include <cstring>
#include <sstream>
#include <cwchar>
#include <charconv>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/unistr.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "localization_manager.h"

std::wstring convertStringToWString(const std::string& str) {
    
    if (str.empty())
    {
        return {};
    }

    size_t pos;
    size_t begin = 0;
    std::wstring ret;
#if defined(CURRENT_OS_WINDOWS)
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed - 1, L'\0'); // -1 to exclude null terminator

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
	ret = wstr;
#elif defined(CURRENT_OS_LINUX) || defined(CURRENT_OS_MACOS)
    size_t size_needed = std::mbstowcs(nullptr, utf8Str.c_str(), 0) + 1;
    std::wstring wstr(size_needed, L'\0');
    std::mbstowcs(&wstr[0], utf8Str.c_str(), size_needed);
	ret = wstr;
#else
    std::runtime_error("Unknown Platform");
#endif
    return ret;
}

uint16_t wstringToUint16(const std::wstring& str)
{
    std::string utf8(str.begin(), str.end()); // Convert wstring to narrow string
    uint16_t value;
    auto [ptr, ec] = std::from_chars(utf8.data(), utf8.data() + utf8.size(), value);
    if (ec == std::errc() && ptr == utf8.data() + utf8.size()) {
        return value;
    }
    else {
        return 0;
    }
}

std::string convertWStringToString(const std::wstring& wstr) {
    
    if (wstr.empty())
    {
        return {};
    }
    size_t pos;
    size_t begin = 0;
    std::string ret;

#if defined(CURRENT_OS_WINDOWS)
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Str(size_needed - 1, '\0'); // -1 to exclude null terminator

    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], size_needed, nullptr, nullptr);
	ret = utf8Str;
#elif defined(CURRENT_OS_LINUX) || defined(CURRENT_OS_MACOS)
    size_t size_needed = std::wcstombs(nullptr, wstr.c_str(), 0) + 1;
    std::string utf8Str(size_needed, '\0');
    std::wcstombs(&utf8Str[0], wstr.c_str(), size_needed);
	ret = utf8Str;
#else
    static_assert(false, "Unknown Platform");
#endif
    return ret;
}

std::u16string LoadStringResourceFor1C(const std::wstring& resourceId)
{
    return LocalizationManager::GetLocalizedStringFor1C(resourceId);
}

std::wstring LoadStringResource(const std::wstring& resourceId)
{
	return LocalizationManager::GetLocalizedString(resourceId);
}

std::wstring u16stringToWstring(const std::u16string& u16str) {
    if (sizeof(wchar_t) == 2) {
        // Windows: wchar_t == UTF-16, можно копировать напрямую
        return std::wstring(u16str.begin(), u16str.end());
    }
    else if (sizeof(wchar_t) == 4) {
        // Linux/macOS: wchar_t == UTF-32, нужна конвертация
        icu::UnicodeString uStr(false, reinterpret_cast<const UChar*>(u16str.data()), static_cast<int32_t>(u16str.length()));

        UErrorCode error = U_ZERO_ERROR;
        int32_t utf32Length = uStr.toUTF32(nullptr, 0, error);  // Получаем длину

        if (error != U_BUFFER_OVERFLOW_ERROR) {
            throw std::runtime_error("UTF-32 string length detection error");
        }

        error = U_ZERO_ERROR;
        std::vector<UChar32> utf32Buffer(utf32Length);
        uStr.toUTF32(utf32Buffer.data(), utf32Length, error);

        if (U_FAILURE(error)) {
            throw std::runtime_error("Error converting to UTF-32");
        }

        return std::wstring(utf32Buffer.begin(), utf32Buffer.end());
    }
    else {
        throw std::runtime_error("Unknown size of wchar_t");
    }
}

std::u16string wstringToU16string(const std::wstring& wstr) {
    if (sizeof(wchar_t) == 2) {
        // Windows: wchar_t == UTF-16, so direct copy is possible
        return std::u16string(wstr.begin(), wstr.end());
    }
    else if (sizeof(wchar_t) == 4) {
        // Linux/macOS: wchar_t == UTF-32, needs conversion using ICU
        UErrorCode status = U_ZERO_ERROR;
        
        int32_t u_char_len;
        std::vector<UChar> u_char(wstr.length() * 2 + 1);
        auto result_uchar = u_strFromUTF32(&u_char[0], u_char.size(), &u_char_len,
            reinterpret_cast<const UChar32*>(wstr.c_str()),
            wstr.length(), &status);

        if (U_FAILURE(status)) {
            return {};
        }

        UConverter* conv = ucnv_open("UTF-16LE", &status);
		if (U_FAILURE(status)) {
            return {};
		}
		
        status = U_ZERO_ERROR;
        int32_t uchar_max_length = u_char_len + 1;
		std::u16string utf16(uchar_max_length, u'\0');
        uint32_t actual_size = ucnv_fromUChars(conv, reinterpret_cast<char*>(&(utf16[0])),
            uchar_max_length, result_uchar, u_char_len, &status);
        utf16.resize(actual_size);

        ucnv_close(conv);

        if (U_FAILURE(status)) {
            return {};
        }

        return utf16;
    }
    else {
        throw std::runtime_error("Unknown wchar_t size.");
    }
}

// Conversion function UTF-8 -> UTF-16
std::u16string utf8ToUtf16(const std::string& source) {
    
    if (source.empty()) {
        return {};
    }

#if defined(CURRENT_OS_WINDOWS)
    int len = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
    if (len <= 0) {
        throw std::runtime_error("Failed to get UTF-16 size.");
    }

    std::u16string utf16(len - 1, u'\0'); // -1 исключает нулевой символ
    MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, reinterpret_cast<LPWSTR>(&utf16[0]), len);

    return utf16;
#else
    iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open failed");
    }

    size_t inSize = utf8.size();
    size_t outSize = inSize * 2 + 2; // UTF-16LE символы занимают минимум 2 байта
    std::u16string utf16(outSize / 2, u'\0');

    char* inBuf = const_cast<char*>(utf8.data());
    char* outBuf = reinterpret_cast<char*>(&utf16[0]);

    size_t result = iconv(cd, &inBuf, &inSize, &outBuf, &outSize);
    iconv_close(cd);

    if (result == (size_t)-1) {
        throw std::runtime_error("iconv failed");
    }

    size_t utf16Len = (outBuf - reinterpret_cast<char*>(&utf16[0])) / 2;
    utf16.resize(utf16Len);

    return utf16;
#endif
}

// Conversion function UTF-16 -> UTF-8
std::u16string convertDriverDescriptionToJson(const DriverDescription& desc) {
	auto ss = std::wstringstream();
    ss << L"\"Name\": \"" << desc.Name << "\", ";
    ss << L"\"Description\": \"" << desc.Description << "\", ";
    ss << L"\"EquipmentType\": \"" << desc.EquipmentType << "\", ";
    ss << L"\"IntegrationComponent\": " << (desc.IntegrationComponent ? "true" : "false") << ", ";
    ss << L"\"MainDriverInstalled\": " << (desc.MainDriverInstalled ? "true" : "false") << ", ";
    ss << L"\"DriverVersion\": \"" << desc.DriverVersion << "\", ";
    ss << L"\"IntegrationComponentVersion\": \"" << desc.IntegrationComponentVersion << "\", ";
    ss << L"\"IsEmulator\": " << (desc.IsEmulator ? "true" : "false") << ", ";
    ss << L"\"LocalizationSupported\": " << (desc.LocalizationSupported ? "true" : "false") << ", ";
    ss << L"\"AutoSetup\": " << (desc.AutoSetup ? "true" : "false") << ", ";
    ss << L"\"DownloadURL\": \"" << desc.DownloadURL << "\", ";
    ss << L"\"EnvironmentInformation\": \"" << desc.EnvironmentInformation << "\", ";
    ss << L"\"LogIsEnabled\": " << (desc.LogIsEnabled ? "true" : "false") << ", ";
    ss << L"\"LogPath\": \"" << desc.LogPath << "\"";
    ss << L"}";
    return wstringToU16string(ss.str());
}
