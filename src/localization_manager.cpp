#include "pch.h"
#include "localization_manager.h"
#include "localization_consts.h"

// Define the static member variables
std::u16string LocalizationManager::lang_code = u"en"; // Default language code

std::unordered_map<
    std::wstring,
    std::unordered_map<std::u16string, LocalizationManager::langEntry>,
    LocalizationManager::WStringHash,
    LocalizationManager::WStringEq
> LocalizationManager::localization_map = {
    {std::wstring{IDS_LC_DRIVER_NAME}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"ECR Driver PrivatBank", L"ECR Driver PrivatBank"} },
        {u"ru", { u"ECR Driver ПриватБанк", L"ECR Driver ПриватБанк"} },
        {u"uk", { u"ECR Driver ПриватБанк", L"ECR Driver ПриватБанк"} }
    }},
    {std::wstring{IDS_LC_DRIVER_DESCRIPTION}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Driver for connecting a payment terminal via ECR protocol PrivatBank protocol (JSON based)",
                  L"Driver for connecting a payment terminal via ECR protocol PrivatBank protocol (JSON based)" }},
        {u"ru", { u"Драйвер подключения платежного терминала по протоколу ECR протокол ПриватБанк (JSON based)",
                  L"Драйвер подключения платежного терминала по протоколу ECR протокол ПриватБанк (JSON based)" }},
        {u"uk", { u"Драйвер підключення платіжного терміналу по протоколу ECR протокол ПриватБанк (JSON based)",
                  L"Драйвер підключення платіжного терміналу по протоколу ECR протокол ПриватБанк (JSON based)" }}
    }},
    {std::wstring{IDS_LC_EQUIPMENT_TYPE}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"POSTerminal", L"POSTerminal" }},
        {u"ru", { u"POSTerminal", L"POSTerminal" }},
        {u"uk", { u"POSTerminal", L"POSTerminal" }}
    }},
    {std::wstring{IDS_LC_DRIVER_VERSION}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"1.0.0", L"1.0.0" }},
        {u"ru", { u"1.0.0", L"1.0.0" }},
        {u"uk", { u"1.0.0", L"1.0.0" }}
    }},
    {std::wstring{IDS_LC_DRIVER_INTEGRATION_COMPONENT_VERSION}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"1.0.0", L"1.0.0" }},
        {u"ru", { u"1.0.0", L"1.0.0" }},
        {u"uk", { u"1.0.0", L"1.0.0" }}
    }},
    {std::wstring{IDS_LC_DRIVER_DOWNLOAD_URL}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"", L"" }},
        {u"ru", { u"", L"" }},
        {u"uk", { u"", L"" }}
    }},
    {std::wstring{IDS_LC_DRIVER_ENVIRONMENT_INFORMATION}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"PrivatBank ECR payment terminal connection driver (JSON-based)\r\nThe package includes executable driver SCODE_ERCDriver.dll",
                  L"PrivatBank ECR payment terminal connection driver (JSON-based)\r\nThe package includes executable driver SCODE_ERCDriver.dll"}},
        {u"ru", { u"Драйвер подключения платежного терминала по протоколу ECR ПриватБанк (JSON-based)\r\nСостав пакета входит файл драйвера SCODE_ERCDriver.dll",
                  L"Драйвер подключения платежного терминала по протоколу ECR ПриватБанк (JSON-based)\r\nСостав пакета входит файл драйвера SCODE_ERCDriver.dll"}},
        {u"uk", { u"Драйвер підключення платіжного терміналу за протоколом ECR ПриватБанк (JSON-based)\r\nСклад пакета входить файл драйверу SCODE_ERCDriver.dll",
                  L"Драйвер підключення платіжного терміналу за протоколом ECR ПриватБанк (JSON-based)\r\nСклад пакета входить файл драйверу SCODE_ERCDriver.dll"}}
    }},
    {std::wstring{IDS_LC_DRIVER_NAME_ADDIN}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"ECRDriverPOS", L"ECRDriverPOS" }},
        {u"ru", { u"ECRDriverPOS", L"ECRDriverPOS" }},
        {u"uk", { u"ECRDriverPOS", L"ECRDriverPOS" }}
    }},
    {std::wstring{IDS_LC_DRIVER_ERROR_CONNECT_TERMINAL}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Failed to connect to POS terminal", L"Failed to connect to POS terminal" }},
        {u"ru", { u"Не удалось подключиться к POS-терминалу", L"Не удалось подключиться к POS-терминалу" }},
        {u"uk", { u"Не вдалося підключитися до POS-терміналу", L"Не вдалося підключитися до POS-терміналу" }}
    }},
    {std::wstring{IDS_LC_DRIVER_ERROR_CONNECT_TIMEOUT}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Connection POS terminal timeout", L"Connection POS terminal timeout" }},
        {u"ru", { u"Тайм-аут подключения к POS-терминалу", L"Тайм-аут подключения к POS-терминалу" }},
        {u"uk", { u"Тайм-аут підключення", L"Тайм-аут підключення" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_CONNECT_PROTOCOL}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Protocol negotiation failed", L"Protocol negotiation failed" }},
        {u"ru", { u"Ошибка согласования протокола", L"Ошибка согласования протокола" }},
        {u"uk", { u"Помилка узгодження протоколу", L"Помилка узгодження протоколу" }}
    }},
};

const std::u16string& LocalizationManager::GetLocalizedStringFor1C(const std::wstring& resourceId)
{
    return localization_map[resourceId][lang_code].u_name;
}

const std::wstring& LocalizationManager::GetLocalizedString(const std::wstring& resourceId)
{
    return localization_map[resourceId][lang_code].w_name;
}

const std::u16string& LocalizationManager::GetLocalizedStringFor1C(const std::wstring_view& resourceId)
{
    auto it = localization_map.find(resourceId);
    if (it == localization_map.end())
        static const std::u16string empty{};
    return it->second[lang_code].u_name;
}

const std::wstring& LocalizationManager::GetLocalizedString(const std::wstring_view& resourceId)
{
    auto it = localization_map.find(resourceId);
    if (it == localization_map.end())
        static const std::wstring empty{};
    return it->second[lang_code].w_name;
}

void LocalizationManager::SetLanguageCode(const std::u16string& langCode)
{
    lang_code = langCode;
}