#include "string_conversion.h"
#include <cstring>
#include <sstream>
#include <cwchar>

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len)
{
    if (!len)
        len = wcslen(Source) + 1;

    *Dest = new WCHAR_T[len];
    for (size_t i = 0; i < len; ++i)
        (*Dest)[i] = (WCHAR_T)Source[i];

    return (uint32_t)len;
}

uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source) + 1;

    *Dest = new wchar_t[len];
    for (uint32_t i = 0; i < len; ++i)
        (*Dest)[i] = (wchar_t)Source[i];

    return len;
}

uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t len = 0;
    while (Source[len] != 0)
        ++len;

    return len;
}

std::wstring convertDriverDescriptionToJson(const DriverDescription& desc) {
	std::wstringstream ss;
    ss << L"{";
    ss << L"\"Name\": \"" << desc.Name << L"\", ";
    ss << L"\"Description\": \"" << desc.Description << L"\", ";
    ss << L"\"EquipmentType\": \"" << desc.EquipmentType << L"\", ";
    ss << L"\"IntegrationComponent\": " << (desc.IntegrationComponent ? L"true" : L"false") << L", ";
    ss << L"\"MainDriverInstalled\": " << (desc.MainDriverInstalled ? L"true" : L"false") << L", ";
    ss << L"\"DriverVersion\": \"" << desc.DriverVersion << L"\", ";
    ss << L"\"IntegrationComponentVersion\": \"" << desc.IntegrationComponentVersion << L"\", ";
    ss << L"\"IsEmulator\": " << (desc.IsEmulator ? L"true" : L"false") << L", ";
    ss << L"\"LocalizationSupported\": " << (desc.LocalizationSupported ? L"true" : L"false") << L", ";
    ss << L"\"AutoSetup\": " << (desc.AutoSetup ? L"true" : L"false") << L", ";
    ss << L"\"DownloadURL\": \"" << desc.DownloadURL << L"\", ";
    ss << L"\"EnvironmentInformation\": \"" << desc.EnvironmentInformation << L"\", ";
    ss << L"\"LogIsEnabled\": " << (desc.LogIsEnabled ? L"true" : L"false") << L", ";
    ss << L"\"LogPath\": \"" << desc.LogPath << L"\"";
    ss << L"}";
    return ss.str();
}
