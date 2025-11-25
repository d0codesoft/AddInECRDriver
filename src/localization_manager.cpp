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
    {std::wstring{IDS_DRIVER_ERROR_DEVICEID_IDENTEFIER}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid Device ID identifier", L"Invalid Device ID identifier" }},
        {u"ru", { u"Недействительный идентификатор устройства", L"Недействительный идентификатор устройства" }},
        {u"uk", { u"Недійсний ідентифікатор пристрою", L"Недійсний ідентифікатор пристрою" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_EQUIPMENTTYPE_IDENTEFIER}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid Equipment type", L"Invalid Equipment type" }},
        {u"ru", { u"Недопустимый тип оборудования", L"Недопустимый тип оборудования" }},
        {u"uk", { u"Недійсний тип обладнання", L"Недійсний тип обладнання" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_CONNECTION_PARAMETERS}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid connection parameters", L"Invalid connection parameters" }},
        {u"ru", { u"Недопустимые параметры соедениия", L"Недопустимые параметры соедениия" }},
        {u"uk", { u"Неприпустимі параметри з'єднання", L"Неприпустимі параметри з'єднання" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_INVALID_ACTION_NAME}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid driver action name", L"Invalid driver action name" }},
        {u"ru", { u"Недопустимое имя действия драйвера", L"Недопустимое имя действия драйвера" }},
        {u"uk", { u"Неприпустиме ім'я дії драйвера", L"Неприпустиме ім'я дії драйвера" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_NOT_FOUND_ACTION_NAME}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Not found driver action name", L"Invalid driver action name" }},
        {u"ru", { u"Не найдено действие драйвера", L"Не найдено действие драйвера" }},
        {u"uk", { u"Не знайдена дія драйвера", L"Не знайдена дія драйвера" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_INVALID_LANGUAGE_CODE}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid language code", L"Invalid language code" }},
        {u"ru", { u"Неверный код языка", L"Неверный код языка" }},
        {u"uk", { u"Помилковий код мови", L"Помилковий код мови" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_UNSUPPORTED_LANGUAGE_CODE}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Driver unsupported language code", L"Driver unsupported language code" }},
        {u"ru", { u"Язык не поддерживается", L"Язык не поддерживается" }},
        {u"uk", { u"Мова не підтримується", L"Мова не підтримується" }}
    }},
    {std::wstring{IDS_DRIVER_ERROR_INVALID_LOCALIZATION_PATTERN}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid localization pattern", L"Invalid localization pattern" }},
        {u"ru", { u"Неверный параметр локализации", L"Неверный параметр локализации" }},
        {u"uk", { u"Неправильний параметр локалізації", L"Неправильний параметр локалізації" }}
    }},
    { std::wstring{IDS_DRIVER_ERROR_INVALID_OPERATION_PARAMETERS}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid operation parameters", L"Invalid operation parameters" }},
        {u"ru", { u"Неверные параметры операции", L"Неверные параметры операции" }},
        {u"uk", { u"Помилкові параметри операції", L"Помилкові параметри операції" }}
    }},
    { std::wstring{IDS_DRIVER_ERROR_INVALID_RESULT_TERMINAL}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid result from POS Terminal", L"Invalid result from POS Terminal" }},
        {u"ru", { u"Неверные параметры ответа POS Терминала", L"Неверные параметры ответа POS Терминала" }},
        {u"uk", { u"Помилкові параметри відповіді POS Термінала", L"Помилкові параметри відповіді POS Термінала" }}
    } },
    { std::wstring{IDS_DRIVER_ERROR_INVALID_OPERATION_TERMINAL}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Invalid operation POS Terminal", L"Invalid operation POS Terminal" }},
        {u"ru", { u"Ошибка выполнения операции POS Терминалом", L"Ошибка выполнения операции POS Терминалом" }},
        {u"uk", { u"Помилка виконання операції POS Терміналом", L"Помилка виконання операції POS Терміналом" }}
    } },
    { std::wstring{IDS_DRIVER_ERROR_OPERATION_NOT_SUPPORTED}, std::unordered_map<std::u16string, LocalizationManager::langEntry>{
        {u"en", { u"Operation not supported", L"Operation not supported" }},
        {u"ru", { u"Не поддерживаемая операция", L"Не поддерживаемая операция" }},
        {u"uk", { u"Не підтримувана операція", L"Не підтримувана операція" }}
    } },
    //
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

void LocalizationManager::AddLocalization(const std::wstring& resourceId,
    const std::u16string& langCode,
    const std::u16string& u_value,
    const std::wstring& w_value)
{
    AddLocalization(std::wstring_view{ resourceId },
        std::u16string_view{ langCode },
        u_value,
        w_value);
}

void LocalizationManager::AddLocalization(const std::wstring_view& resourceId,
    const std::u16string_view& langCode,
    const std::u16string& u_value,
    const std::wstring& w_value)
{
    // Ensure resource entry exists
    auto& langMap = localization_map[std::wstring{ resourceId }];
    // Insert or update language entry
    auto& entry = langMap[std::u16string{ langCode }];
    entry.u_name = u_value;
    entry.w_name = w_value;
}