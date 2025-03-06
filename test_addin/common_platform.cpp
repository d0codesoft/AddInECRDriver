#include "common_platform.h"
#include <iomanip>

// Defining a platform-specific library name
std::wstring getLibraryName() {
#ifdef _WIN64
    return L"ECR_Driver_PT_win64" LIB_EXTENSION;
#elif defined(_WIN32)
    return L"ECR_Driver_PT_win32" LIB_EXTENSION;
#elif defined(__APPLE__)
#undef LIB_EXTENSION
#define LIB_EXTENSION ".dylib"
    return L"ECR_Driver_PT_macos" LIB_EXTENSION;
#else
    return L"ECR_Driver_PT_linux" LIB_EXTENSION;
#endif
}

void printConsole(const std::wstring& text) {
#ifdef _WIN32
    DWORD written;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), static_cast<DWORD>(text.size()), &written, NULL);
#else
    // На Linux/macOS можно просто использовать std::wcout
    std::wcout << text;
#endif
}


std::wstring toWString(const std::u16string& u16str) {
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

std::wstring toWString(const std::string& str)
{
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str);

    std::wstring wstr;
    int32_t requiredSize;
    UErrorCode error = U_ZERO_ERROR;

    u_strToWCS(nullptr, 0, &requiredSize, ustr.getBuffer(), ustr.length(), &error);
	if (error != U_BUFFER_OVERFLOW_ERROR) {
		throw std::runtime_error("Error getting the required size for conversion");
	}

    wstr.resize(requiredSize);

    u_strToWCS(wstr.data(), wstr.size(), nullptr, ustr.getBuffer(), ustr.length(), &error);
	if (U_FAILURE(error)) {
		throw std::runtime_error("Error converting to wide string");
	}
	return wstr;
}

std::wstring getVariantValue(const tVariant& variant)
{
    std::wstringstream wss;
    wss << L"";
    switch (TV_VT(&variant)) {
    case VTYPE_I1:
        wss << static_cast<int>(TV_I1(&variant)) << L" (int8)";
        break;
    case VTYPE_I2:
        wss << TV_I2(&variant) << L" (int16)";
        break;
    case VTYPE_I4:
        wss << TV_I4(&variant) << L" (int32)";
        break;
    case VTYPE_INT:
        wss << TV_INT(&variant) << L" (int)";
        break;
    case VTYPE_UI1:
        wss << static_cast<unsigned int>(TV_UI1(&variant)) << L" (uint8)";
        break;
    case VTYPE_UI2:
        wss << TV_UI2(&variant) << L" (uint16)";
        break;
    case VTYPE_UI4:
        wss << TV_UI4(&variant) << L" (uint32)";
        break;
    case VTYPE_UINT:
        wss << TV_UINT(&variant) << L" (uint)";
        break;
    case VTYPE_I8:
        wss << TV_I8(&variant) << L" (int64)";
        break;
    case VTYPE_UI8:
        wss << TV_UI8(&variant) << L" (uint64)";
        break;
    case VTYPE_ERROR:
        wss << TV_JOIN(&variant, errCode) << L" (error)";
        break;
    case VTYPE_HRESULT:
        wss << TV_JOIN(&variant, hRes) << L" (HRESULT)";
        break;
    case VTYPE_R4:
        wss << TV_R4(&variant) << L" (float)";
        break;
    case VTYPE_R8:
        wss << TV_R8(&variant) << L" (double)";
        break;
    case VTYPE_BOOL:
        wss << (TV_BOOL(&variant) ? L"true" : L"false");
        break;
    case VTYPE_PSTR:
        wss << toWString(std::string(TV_STR(&variant))) << L" (PSTR)";
        break;
    case VTYPE_PWSTR:
        wss << toWString(TV_WSTR(&variant)) << L" (PWSTR)";
        break;
    case VTYPE_DATE:
        wss << TV_DATE(&variant) << L" (date)";
        break;
    case VTYPE_TM:
        auto val = TV_JOIN(&variant, tmVal);
        wss << std::put_time(&val, L"%c") << L" (time)";
        break;
    default:
        wss << L"";
        break;
    }
    return wss.str();
}

SharedLibrary::SharedLibrary(const std::wstring& libName) : handle(LoadSharedLibrary(libName.c_str())) {
    if (!handle) {
        std::wcout << L"Failed to load the library: " << libName;
    }
}

SharedLibrary::~SharedLibrary()
{
    if (handle) {
        CloseSharedLibrary(handle);
    }
}
