#include "pch.h"
#include "string_conversion.h"
#include <cstring>
#include <sstream>
#include <cwchar>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <string>
#include <vector>
#include <stdexcept>

// Function to load a string resource
std::string LoadStringResource(UINT resourceId) {
    wchar_t buffer[256] = { 0 };

    // Получаем текущий язык пользователя
    //LANGID langId = GetUserDefaultUILanguage(); // Можно заменить на GetThreadUILanguage()

    // Загружаем строку для текущего языка
    HMODULE hModule = GetModuleHandle(NULL);
    int length = LoadStringW(hModule, resourceId, buffer, sizeof(buffer) / sizeof(buffer[0]));

    // Если строка не найдена, пробуем загрузить английскую версию (как запасной вариант)
    if (length == 0) {
        length = LoadStringW(hModule, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), buffer, sizeof(buffer) / sizeof(buffer[0]));
    }

	if (length == 0) {
		return {};
	}
	return wcharToString(buffer);
}

// Conversion function UTF-16 -> UTF-8
std::string utf16ToUtf8(const std::u16string& utf16) {
    UErrorCode error = U_ZERO_ERROR;
    UConverter* conv = ucnv_open("UTF-8", &error);
    if (U_FAILURE(error)) return {};

    int32_t utf8Len = ucnv_fromUChars(conv, nullptr, 0, reinterpret_cast<const UChar*>(utf16.c_str()), utf16.length(), &error);
    if (error != U_BUFFER_OVERFLOW_ERROR) {
        ucnv_close(conv);
        return {};
    }

    error = U_ZERO_ERROR;
    std::string utf8(utf8Len, 0);
    ucnv_fromUChars(conv, utf8.data(), utf8Len, reinterpret_cast<const UChar*>(utf16.c_str()), utf16.length(), &error);
    ucnv_close(conv);

    return utf8;
}

// Функция конвертации WCHAR_T* -> wchar_t* (UTF-16)
std::string convFromShortToString(const WCHAR_T* Source, uint32_t len) {
    return utf16ToUtf8(std::u16string(Source, len));
}

// Conversion function UTF-8 -> UTF-16
std::u16string utf8ToUtf16(const std::string& utf8) {
    UErrorCode error = U_ZERO_ERROR;
    UConverter* conv = ucnv_open("UTF-16LE", &error);
    if (U_FAILURE(error)) return {};

    int32_t utf16Len = ucnv_toUChars(conv, nullptr, 0, utf8.c_str(), utf8.length(), &error);
    if (error != U_BUFFER_OVERFLOW_ERROR) {
        ucnv_close(conv);
        return {};
    }

    error = U_ZERO_ERROR;
    std::u16string utf16(utf16Len, 0);
    ucnv_toUChars(conv, reinterpret_cast<UChar*>(utf16.data()), utf16Len, utf8.c_str(), utf8.length(), &error);
    ucnv_close(conv);

    return utf16;
}

std::string wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};

    UErrorCode status = U_ZERO_ERROR;
    int32_t utf8Length = 0;

    // Определяем размер буфера для UTF-8
    u_strToUTF8(nullptr, 0, &utf8Length, reinterpret_cast<const UChar*>(wstr.data()), wstr.size(), &status);

    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        throw std::runtime_error("ICU: u_strToUTF8 failed");
    }

    status = U_ZERO_ERROR;
    std::vector<char> utf8Buffer(utf8Length);

    // Конвертация в UTF-8
    u_strToUTF8(utf8Buffer.data(), utf8Length, nullptr, reinterpret_cast<const UChar*>(wstr.data()), wstr.size(), &status);

    if (U_FAILURE(status)) {
        throw std::runtime_error("ICU: u_strToUTF8 conversion failed");
    }

    return std::string(utf8Buffer.begin(), utf8Buffer.end());
}

// Конвертация std::string (UTF-8) → std::wstring (UTF-16)
std::wstring utf8ToWstring(const std::string& str) {
    if (str.empty()) return {};

    UErrorCode status = U_ZERO_ERROR;
    UConverter* converter = ucnv_open("UTF-8", &status);
    if (U_FAILURE(status)) {
        throw std::runtime_error("Ошибка открытия ICU конвертера");
    }

    // Подсчет нужного размера выходного буфера
    int32_t utf16Len = ucnv_toUChars(converter, nullptr, 0, str.c_str(), str.size(), &status);
    if (status != U_BUFFER_OVERFLOW_ERROR) {
        ucnv_close(converter);
        throw std::runtime_error("Ошибка при расчете размера строки");
    }

    status = U_ZERO_ERROR;
    std::vector<wchar_t> utf16Buffer(utf16Len);
    ucnv_toUChars(converter, reinterpret_cast<UChar*>(utf16Buffer.data()), utf16Len, str.c_str(), str.size(), &status);
    ucnv_close(converter);

    if (U_FAILURE(status)) {
        throw std::runtime_error("Ошибка конвертации в UTF-16");
    }

    return std::wstring(utf16Buffer.begin(), utf16Buffer.end());
}

std::string wcharToString(const wchar_t* wstr) {
    if (!wstr) return "";

    UErrorCode err = U_ZERO_ERROR;
    UConverter* conv = ucnv_open("UTF-8", &err);
    if (U_FAILURE(err)) return "";

    int32_t len = ucnv_fromUChars(conv, nullptr, 0, reinterpret_cast<const UChar*>(wstr), -1, &err);
    err = U_ZERO_ERROR;  // Сброс ошибки
    std::vector<char> buffer(len);
    ucnv_fromUChars(conv, buffer.data(), len, reinterpret_cast<const UChar*>(wstr), -1, &err);
    ucnv_close(conv);

    return std::string(buffer.data(), buffer.size() - 1);
}

std::string convertDriverDescriptionToJson(const DriverDescription& desc) {
	std::stringstream ss;
    ss << "{";
    ss << "\"Name\": \"" << desc.Name << "\", ";
    ss << "\"Description\": \"" << desc.Description << "\", ";
    ss << "\"EquipmentType\": \"" << desc.EquipmentType << "\", ";
    ss << "\"IntegrationComponent\": " << (desc.IntegrationComponent ? "true" : "false") << ", ";
    ss << "\"MainDriverInstalled\": " << (desc.MainDriverInstalled ? "true" : "false") << ", ";
    ss << "\"DriverVersion\": \"" << desc.DriverVersion << "\", ";
    ss << "\"IntegrationComponentVersion\": \"" << desc.IntegrationComponentVersion << "\", ";
    ss << "\"IsEmulator\": " << (desc.IsEmulator ? "true" : "false") << ", ";
    ss << "\"LocalizationSupported\": " << (desc.LocalizationSupported ? "true" : "false") << ", ";
    ss << "\"AutoSetup\": " << (desc.AutoSetup ? "true" : "false") << ", ";
    ss << "\"DownloadURL\": \"" << desc.DownloadURL << "\", ";
    ss << "\"EnvironmentInformation\": \"" << desc.EnvironmentInformation << "\", ";
    ss << "\"LogIsEnabled\": " << (desc.LogIsEnabled ? "true" : "false") << ", ";
    ss << "\"LogPath\": \"" << desc.LogPath << "\"";
    ss << "}";
    return ss.str();
}

//// Function to convert const WCHAR_T* to char*
//char* convertWCharToChar(const WCHAR_T* wstr) {
//
//    wchar_t* result = nullptr;
//    convFromShortWchar(&result, wstr, 0);
//	//convFromShortWchar(&wstr, wstr);
//    // Get the length of the wide string
//    size_t wlen = wcslen(result);
//
//    // Allocate memory for the multibyte string
//    size_t len = wlen * MB_CUR_MAX + 1;
//    char* str = (char*)malloc(len);
//
//    if (str) {
//        // Convert the wide string to a multibyte string
//        wcstombs(str, result, len);
//    }
//
//	delete[] result;
//
//    return str;
//}
