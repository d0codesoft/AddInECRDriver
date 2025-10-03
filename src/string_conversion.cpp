#include "pch.h"
#include "string_conversion.h"
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "localization_manager.h"
#include "str_utils.h"

std::u16string LoadStringResourceFor1C(const std::wstring& resourceId)
{
    return LocalizationManager::GetLocalizedStringFor1C(resourceId);
}

std::wstring LoadStringResource(const std::wstring& resourceId)
{
	return LocalizationManager::GetLocalizedString(resourceId);
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
    return str_utils::to_u16string(ss.str());
}
