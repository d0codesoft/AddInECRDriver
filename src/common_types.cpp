#include "pch.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "pugixml.hpp"
#include "common_types.h"
#include "string_conversion.h"

bool isValidEquipmentType(const std::u16string& input) {
    return std::any_of(EquipmentTypes.begin(), EquipmentTypes.end(), [&](const EquipmentType& eq) {
        return eq.english == input || eq.russian == input;
        });
}

// 🌐 Language definition: Russian or English
std::u16string detectLanguage(const std::u16string& input) {
    if (std::any_of(EquipmentTypes.begin(), EquipmentTypes.end(), [&](const EquipmentType& eq) {
        return eq.english == input;
        })) return u"EN";

    if (std::any_of(EquipmentTypes.begin(), EquipmentTypes.end(), [&](const EquipmentType& eq) {
        return eq.russian == input;
        })) return u"RU";

    return u"Unknown";
}

// 🏷️ Search for English analogs in Russian
std::u16string findEquivalent(const std::u16string& input) {
    for (const auto& eq : EquipmentTypes) {
        if (eq.english == input) return eq.russian;
        if (eq.russian == input) return eq.english;
    }
    return u"Not Found";
}

std::optional<EquipmentTypeInfo> getEquipmentTypeInfo(const std::u16string& input)
{
    for (const auto& type : EquipmentTypes) {
        if (type.english == input || type.russian == input) {
            return type.type;
        }
    }
    return std::nullopt;
}

// 📝 XML file parsing function
std::vector<DriverParameter> ParseParameters(const std::wstring& xmlPath) {
    std::vector<DriverParameter> parameters;
    pugi::xml_document doc;

    // 📂 Загрузка XML файла
    pugi::xml_parse_result result = doc.load_file(xmlPath.c_str(), pugi::parse_default, pugi::encoding_utf8);
    if (!result) {
        std::wcerr << L"Ошибка загрузки XML: " << result.description() << std::endl;
        return parameters;
    }

    // 🔍 Поиск всех узлов Parameter
    for (pugi::xml_node paramNode : doc.child(L"Parameters").children(L"Parameter")) {
        DriverParameter param;
        param.name = paramNode.attribute(L"Name").as_string();
        param.value = paramNode.attribute(L"Value").as_string();

        // ✂️ Удаляем лишние пробелы и переносы строк
        param.name.erase(remove_if(param.name.begin(), param.name.end(), iswspace), param.name.end());
        param.value.erase(remove_if(param.value.begin(), param.value.end(), iswspace), param.value.end());

        parameters.push_back(param);
    }

    return parameters;
}

std::optional<std::wstring> findParameterValue(
    const std::vector<DriverParameter>& params, const std::wstring& paramName)
{
    auto it = std::ranges::find_if(params, [&paramName](const DriverParameter& p) {
        return p.name == paramName;
        });

    if (it != params.end()) {
        return it->value;
    }
    return std::nullopt;
}

std::u16string toXml(const DriverDescription& driver)
{
    pugi::xml_document doc;
    auto decl = doc.append_child(pugi::node_declaration);
    decl.append_attribute(L"version") = "1.0";
    decl.append_attribute(L"encoding") = "UTF-8";

    auto root = doc.append_child(L"DriverDescription");
    root.append_attribute(L"Name") = driver.Name.c_str();
    root.append_attribute(L"Description") = driver.Description.c_str();
    root.append_attribute(L"EquipmentType") = driver.EquipmentType.c_str();
    root.append_attribute(L"IntegrationComponent") = BOOL_TO_STRING(driver.IntegrationComponent);
    root.append_attribute(L"MainDriverInstalled") = BOOL_TO_STRING(driver.MainDriverInstalled);
    root.append_attribute(L"DriverVersion") = driver.DriverVersion.c_str();
    root.append_attribute(L"IntegrationComponentVersion") = driver.IntegrationComponentVersion.c_str();
    root.append_attribute(L"IsEmulator") = BOOL_TO_STRING(driver.IsEmulator);
    root.append_attribute(L"LocalizationSupported") = BOOL_TO_STRING(driver.LocalizationSupported);
    root.append_attribute(L"AutoSetup") = BOOL_TO_STRING(driver.AutoSetup);
    root.append_attribute(L"DownloadURL") = driver.DownloadURL.c_str();
    root.append_attribute(L"EnvironmentInformation") = driver.EnvironmentInformation.c_str();
    root.append_attribute(L"LogIsEnabled") = BOOL_TO_STRING(driver.LogIsEnabled);
    root.append_attribute(L"LogPath") = driver.LogPath.c_str();
    root.append_attribute(L"ExtensionName") = driver.ExtensionName.c_str();

    std::wostringstream oss;
    doc.save(oss);

    std::wstring xml_str = oss.str();
    return wstringToU16string(xml_str);
}
