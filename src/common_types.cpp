#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include "pugixml.hpp"
#include "common_types.h"

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