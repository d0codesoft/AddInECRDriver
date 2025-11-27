#include "common_platform.h"
#include <iomanip>

void printConsole(const std::wstring& text) {
#ifdef _WIN32
    DWORD written;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), static_cast<DWORD>(text.size()), &written, NULL);
#else
    // На Linux/macOS можно просто использовать std::wcout
    std::wcout << text;
#endif
}

std::wstring getLibraryName()
{
    return std::wstring();
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
        wss << str_utils_test::to_wstring(std::string(TV_STR(&variant))) << L" (PSTR)";
        break;
    case VTYPE_PWSTR:
        wss << str_utils_test::to_wstring(TV_WSTR(&variant)) << L" (PWSTR)";
        break;
    case VTYPE_DATE:
        wss << TV_DATE(&variant) << L" (date)";
        break;
    case VTYPE_TM:
        auto _tmVal = variant.tmVal;
        wss << std::put_time(&_tmVal, L"%c") << L" (time)";
        break;
    }
    return wss.str();
}

SharedLibrary::SharedLibrary(const std::wstring& libName) {
#ifdef _WIN32
    handle = LoadSharedLibrary(libName.c_str());
#else
    std::string narrowLibName = str_utils_test::to_string(libName);
    handle = LoadSharedLibrary(narrowLibName.c_str());
#endif

    if (!handle) {
        std::wcout << L"Failed to load the library: " << libName << std::endl;
    }
}

SharedLibrary::~SharedLibrary()
{
    if (handle) {
        CloseSharedLibrary(handle);
    }
}
