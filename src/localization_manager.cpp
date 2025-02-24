#include "pch.h"
#include "localization_manager.h"

// Define the static member variables
std::u16string LocalizationManager::lang_code = u"en"; // Default language code
std::unordered_map<std::wstring, std::unordered_map<std::u16string, std::u16string>> LocalizationManager::localization_map = {
    {L"IDS_DRIVER_NAME", {
        {u"en", u"ECR Driver PrivatBank"},
        {u"ru", u"ECR Driver ПриватБанк"},
        {u"uk", u"ECR Driver ПриватБанк"}
    }},
    {L"IDS_DRIVER_DESCRIPTION", {
        {u"en", u"Driver for connecting a payment terminal via ECR protocol PrivatBank protocol (JSON based)"},
        {u"ru", u"Драйвер подключения платежного терминала по протоколу ECR протокол ПриватБанк (JSON based)"},
		{u"uk", u"Драйвер підключення платіжного терміналу по протоколу ECR протокол ПриватБанк (JSON based)"}
    }},
    {L"IDS_EQUIPMENT_TYPE", {
        {u"en", u"POSTerminal"},
        {u"ru", u"POSTerminal"},
        {u"uk", u"POSTerminal"}
    }},
    {L"IDS_DRIVER_VERSION", {
        {u"en", u"1.0.0"},
        {u"ru", u"1.0.0"},
        {u"uk", u"1.0.0" }
    }},
    {L"IDS_DRIVER_INTEGRATION_COMPONENT_VERSION", {
        {u"en", u"1.0.0"},
        {u"ru", u"1.0.0"},
        {u"uk", u"1.0.0" }
    }},
    {L"IDS_DRIVER_DOWNLOAD_URL", {
        {u"en", u""},
        {u"ru", u""},
        {u"uk", u""}
    }},
    {L"IDS_DRIVER_ENVIRONMENT_INFORMATION", {
        {u"en", u"PrivatBank ECR payment terminal connection driver (JSON-based)\r\nThe package includes executable driver SCODE_ERCDriver.dll"},
        {u"ru", u"Драйвер подключения платежного терминала по протоколу ECR ПриватБанк (JSON-based)\r\nСостав пакета входит файл драйвера SCODE_ERCDriver.dll"},
		{u"uk", u"Драйвер підключення платіжного термінала за протоколом ECR ПриватБанк (JSON-based)\r\nСклад пакета входить файл драйверу SCODE_ERCDriver.dll"}
    }},
    {L"IDS_DRIVER_NAME_ADDIN", {
        {u"en", u"ECRDriverPOS"},
        {u"ru", u"ECRDriverPOS"},
        {u"uk", u"ECRDriverPOS"}
    }}
};

std::unordered_map<std::wstring, std::unordered_map<std::u16string, std::wstring>> LocalizationManager::localization_map_ws = {
    {L"IDS_DRIVER_NAME", {
        {u"en", L"ECR Driver PrivatBank"},
        {u"ru", L"ECR Driver ПриватБанк"},
        {u"uk", L"ECR Driver ПриватБанк"}
    }},
    {L"IDS_DRIVER_DESCRIPTION", {
        {u"en", L"Driver for connecting a payment terminal via ECR protocol PrivatBank protocol (JSON based)"},
        {u"ru", L"Драйвер подключения платежного терминала по протоколу ECR протокол ПриватБанк (JSON based)"},
        {u"uk", L"Драйвер підключення платіжного терміналу по протоколу ECR протокол ПриватБанк (JSON based)"}
    }},
    {L"IDS_EQUIPMENT_TYPE", {
        {u"en", L"POSTerminal"},
        {u"ru", L"POSTerminal"},
        {u"uk", L"POSTerminal"}
    }},
    {L"IDS_DRIVER_VERSION", {
        {u"en", L"1.0.0"},
        {u"ru", L"1.0.0"},
        {u"uk", L"1.0.0" }
    }},
    {L"IDS_DRIVER_INTEGRATION_COMPONENT_VERSION", {
        {u"en", L"1.0.0"},
        {u"ru", L"1.0.0"},
        {u"uk", L"1.0.0" }
    }},
    {L"IDS_DRIVER_DOWNLOAD_URL", {
        {u"en", L""},
        {u"ru", L""},
        {u"uk", L""}
    }},
    {L"IDS_DRIVER_ENVIRONMENT_INFORMATION", {
        {u"en", L"PrivatBank ECR payment terminal connection driver (JSON-based)\r\nThe package includes executable driver SCODE_ERCDriver.dll"},
        {u"ru", L"Драйвер подключения платежного терминала по протоколу ECR ПриватБанк (JSON-based)\r\nСостав пакета входит файл драйвера SCODE_ERCDriver.dll"},
        {u"uk", L"Драйвер підключення платіжного термінала за протоколом ECR ПриватБанк (JSON-based)\r\nСклад пакета входить файл драйверу SCODE_ERCDriver.dll"}
    }},
    {L"IDS_DRIVER_NAME_ADDIN", {
        {u"en", L"ECRDriverPOS"},
        {u"ru", L"ECRDriverPOS"},
        {u"uk", L"ECRDriverPOS"}
    }}
};

const std::u16string& LocalizationManager::GetLocalizedStringFor1C(const std::wstring& resourceId)
{
    return localization_map[resourceId][lang_code];
}

const std::wstring& LocalizationManager::GetLocalizedString(const std::wstring& resourceId)
{
    return localization_map_ws[resourceId][lang_code];
}

void LocalizationManager::SetLanguageCode(const std::u16string& langCode)
{
    lang_code = langCode;
}