#include "pch.h"
#include "string_conversion.h"
#include <cstring>
#include <sstream>
#include <cwchar>
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
#if defined(OS_WINDOWS)
    int size = 0;
    pos = str.find(static_cast<char>(0), begin);
    while (pos != std::string::npos) {
        std::string segment = std::string(&str[begin], pos - begin);
        std::wstring converted = std::wstring(segment.size() + 1, 0);
        size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.length());
        converted.resize(size);
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = str.find(static_cast<char>(0), begin);
    }
    if (begin < str.length()) {
        std::string segment = std::string(&str[begin], str.length() - begin);
        std::wstring converted = std::wstring(segment.size() + 1, 0);
        size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, segment.c_str(), segment.size(), &converted[0], converted.length());
        converted.resize(size);
        ret.append(converted);
    }

#elif defined(OS_LINUX) || defined(OS_MACOS)
    size_t size;
    pos = str.find(static_cast<char>(0), begin);
    while (pos != String::npos)
    {
        std::string segment = std::string(&str[begin], pos - begin);
        std::wstring converted = std::wstring(segment.size(), 0);
        size = mbstowcs(&converted[0], &segment[0], converted.size());
        converted.resize(size);
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = str.find(static_cast<char>(0), begin);
    }
    if (begin < str.length())
    {
        std::string segment = std::string(&str[begin], str.length() - begin);
        std::wstring converted = std::wstring(segment.size(), 0);
        size = mbstowcs(&converted[0], &segment[0], converted.size());
        converted.resize(size);
        ret.append(converted);
    }
#else
    std::runtime_error("Unknown Platform");
#endif
    return ret;
}

std::string convertWStringToString(const std::wstring& wstr) {
    
    if (wstr.empty())
    {
        return {};
    }
    size_t pos;
    size_t begin = 0;
    std::string ret;

#if defined(OS_WINDOWS)
    int size;
    pos = wstr.find(static_cast<wchar_t>(0), begin);
    while (pos != std::wstring::npos && begin < wstr.length())
    {
        std::wstring segment = std::wstring(&wstr[begin], pos - begin);
        size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), NULL, 0, NULL, NULL);
        std::string converted = std::string(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.size(), NULL, NULL);
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = wstr.find(static_cast<wchar_t>(0), begin);
    }
    if (begin <= wstr.length())
    {
        std::wstring segment = std::wstring(&wstr[begin], wstr.length() - begin);
        size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), NULL, 0, NULL, NULL);
        std::string converted = std::string(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.size(), NULL, NULL);
        ret.append(converted);
    }
#elif defined(OS_LINUX) || defined(OS_MACOS)
    size_t size;
    pos = wstr.find(static_cast<wchar_t>(0), begin);
    while (pos != WString::npos && begin < wstr.length())
    {
        WString segment = WString(&wstr[begin], pos - begin);
        size = wcstombs(nullptr, segment.c_str(), 0);
        String converted = String(size, 0);
        wcstombs(&converted[0], segment.c_str(), converted.size());
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = wstr.find(static_cast<wchar_t>(0), begin);
    }
    if (begin <= wstr.length())
    {
        WString segment = WString(&wstr[begin], wstr.length() - begin);
        size = wcstombs(nullptr, segment.c_str(), 0);
        String converted = String(size, 0);
        wcstombs(&converted[0], segment.c_str(), converted.size());
        ret.append(converted);
    }
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

#if defined(OS_WINDOWS)
    int len = MultiByteToWideChar(CP_ACP, 0, source.c_str(), -1, nullptr, 0);
    if (len <= 0) {
        throw std::runtime_error("Failed to get UTF-16 size.");
    }

    std::u16string utf16(len - 1, u'\0'); // -1 исключает нулевой символ
    MultiByteToWideChar(CP_ACP, 0, source.c_str(), -1, reinterpret_cast<LPWSTR>(&utf16[0]), len);

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
