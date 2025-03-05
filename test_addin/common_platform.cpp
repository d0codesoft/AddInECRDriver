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

inline std::wstring getVariantValue(const tVariant& variant)
{
    std::wstringstream wss;
    wss << L"";
    switch (TV_VT(&variant)) {
    case VTYPE_I1:
        wss << L"int8 = " << static_cast<int>(TV_I1(&variant));
        break;
    case VTYPE_I2:
        wss << L"short int = " << TV_I2(&variant);
        break;
    case VTYPE_I4:
        wss << L"int4 = " << TV_I4(&variant);
        break;
    case VTYPE_INT:
        wss << L"int = " << TV_INT(&variant);
        break;
    case VTYPE_UI1:
        wss << L"u8int = " << static_cast<unsigned int>(TV_UI1(&variant));
        break;
    case VTYPE_UI2:
        wss << L"ushort = " << TV_UI2(&variant);
        break;
    case VTYPE_UI4:
        wss << L"u4int = " << TV_UI4(&variant);
        break;
    case VTYPE_UINT:
        wss << L"int = " << TV_UINT(&variant);
        break;
    case VTYPE_I8:
        wss << L"long = " << TV_I8(&variant);
        break;
    case VTYPE_UI8:
        wss << L"ulong = " << TV_UI8(&variant);
        break;
    case VTYPE_ERROR:
        wss << L"error = " << TV_JOIN(&variant, errCode);
        break;
    case VTYPE_HRESULT:
        wss << L"HRESULT = " << TV_JOIN(&variant, hRes);
        break;
    case VTYPE_R4:
        wss << L"float = " << TV_R4(&variant);
        break;
    case VTYPE_R8:
        wss << L"double = " << TV_R8(&variant);
        break;
    case VTYPE_BOOL:
        wss << L"bool = " << (TV_BOOL(&variant) ? L"true" : L"false");
        break;
    case VTYPE_PSTR:
        wss << L"PSTR = " << toWString(std::string(TV_STR(&variant)));
        break;
    case VTYPE_PWSTR:
        wss << L"PWSTR = " << toWString(TV_WSTR(&variant));
        break;
    case VTYPE_DATE:
        wss << L"date = " << TV_DATE(&variant);
        break;
    case VTYPE_TM:
        auto val = TV_JOIN(&variant, tmVal);
        wss << L"Time = " << std::put_time(&val, L"%c");
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
