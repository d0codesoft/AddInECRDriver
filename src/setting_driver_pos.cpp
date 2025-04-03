#include "pch.h"
#include "setting_driver_pos.h"
#include <pugixml.hpp>
#include <sstream>
#include "string_conversion.h"
#include "str_utils.h"


const SettingSettings SettingDriverPos::m_settings = {
{
    {
        L"Параметры", // PageCaption
        {
            {   // Group Connection parameters
                L"Параметры подключения",
                {
                    {L"ConnectionType", L"Тип подключения", L"Выберите тип подключения.", L"Number", L"", L"0", false,
                     {{L"0", L"TCP"}, {L"1", L"COM"}, {L"2", L"WebSocket"}}},

                    {L"Address", L"Адрес подключения", L"Введите адрес сервера.", L"String", L"", L"", false, {}},

                    {L"Port", L"Порт", L"Введите номер порта.", L"Number", L"", L"2000", false, {}}

                    //{L"Speed", L"Скорость подключения", L"Укажите скорость соединения.", L"Number", L"", L"9600", false, {}}
                }
            },
			{   // Group payement parameters
				L"Параметры оплаты",
				{
					{L"Facepay", L"Разрешить оплату через FacePay24", L"", L"Boolean", L"", L"false", false,{}},
					{L"MerchantId", L"Код мерчанта", L"Введите код мерчанта", L"String", L"", L"", false,{}},
				}
			},
			{   // Logging parameters
				L"Логирование",
				{
					{L"LogFullPath", L"Текущий файл лога", L"", L"String", L"", L"", true, {}},
					{L"LogLevel", L"Уроверь логирования", L"Выбирите уровень логирования", L"Number", L"", L"0", false,
					 {{L"0", L"Ошибки"}, {L"1", L"Детальный"}}},
				}
			},
            {   // Group licensing
                L"Лицензирование",
                {
                    {L"DriverVersion", L"Версия драйвера", L"", L"String", L"", L"1.0.0", true, {}},
                    {L"LicenseStatus", L"Статус лицензии", L"", L"String", L"", L"Не активирована", true, {}},
                    { L"LicenseKey", L"Ключ лицензии", L"Введите ключ лицензии.", L"String", L"", L"", false, {} }
                }
            }
        }
    }
}
};

const SettingSettings& SettingDriverPos::getSettings()
{
	return m_settings;
}

std::u16string SettingDriverPos::getSettingXML()
{
    pugi::xml_document doc;
    auto decl = doc.append_child(pugi::node_declaration);
    decl.append_attribute(L"version") = L"1.0";
    decl.append_attribute(L"encoding") = L"UTF-8";

    auto root = doc.append_child(L"Settings");

    for (const auto& page : m_settings.pages) {
        auto pageNode = root.append_child(L"Page");
        pageNode.append_attribute(L"Caption") = page.Caption.c_str();

        for (const auto& group : page.Groups) {
            auto groupNode = pageNode.append_child(L"Group");
            groupNode.append_attribute(L"Caption") = group.Caption.c_str();

            for (const auto& param : group.Parameters) {
                auto paramNode = groupNode.append_child(L"Parameter");
                paramNode.append_attribute(L"Name") = param.Name.c_str();
                paramNode.append_attribute(L"Caption") = param.Caption.c_str();
                paramNode.append_attribute(L"TypeValue") = param.TypeValue.c_str();
                if (!param.FieldFormat.empty())
                    paramNode.append_attribute(L"FieldFormat") = param.FieldFormat.c_str();
                if (!param.DefaultValue.empty())
                    paramNode.append_attribute(L"DefaultValue") = param.DefaultValue.c_str();
                if (param.ReadOnly)
                    paramNode.append_attribute(L"ReadOnly") = L"true";

                if (!param.ChoiceList.empty()) {
                    auto choiceListNode = paramNode.append_child(L"ChoiceList");
                    for (const auto& item : param.ChoiceList) {
                        auto itemNode = choiceListNode.append_child(L"Item");
                        itemNode.append_attribute(L"Value") = item.Value.c_str();
                        itemNode.text() = item.DisplayName.c_str();
                    }
                }
            }
        }
    }

    std::wostringstream oss;
    doc.save(oss);
    return str_utils::to_u16string(oss.str());
}

std::u16string toXML(const SettingSettings& settings, std::span<const DriverParameter> parameters) {
	pugi::xml_document doc;
	auto decl = doc.append_child(pugi::node_declaration);
	decl.append_attribute(L"version") = L"1.0";
	decl.append_attribute(L"encoding") = L"UTF-8";

	auto root = doc.append_child(L"Settings");

	for (const auto& page : settings.pages) {
		auto pageNode = root.append_child(L"Page");
		pageNode.append_attribute(L"Caption") = page.Caption;

		for (const auto& group : page.Groups) {
			auto groupNode = pageNode.append_child(L"Group");
			groupNode.append_attribute(L"Caption") = group.Caption;

			for (const auto& param : group.Parameters) {
				auto paramNode = groupNode.append_child(L"Parameter");
				paramNode.append_attribute(L"Name") = param.Name;
				paramNode.append_attribute(L"Caption") = param.Caption;
				paramNode.append_attribute(L"TypeValue") = param.TypeValue;
				paramNode.append_attribute(L"ReadOnly") = (param.ReadOnly) ? L"true" : L"false";
				if (param.ReadOnly) {
					auto valDefault = std::find_if(parameters.begin(), parameters.end(), [&](const DriverParameter& p) {
						return p.name == param.Name;
						});
					if (valDefault != parameters.end()) {
                        const std::wstring* strValue = std::get_if<std::wstring>(&valDefault->value);
						paramNode.append_attribute(L"DefaultValue") = std::wstring(strValue->begin(), strValue->end());
					}
                    else {
						paramNode.append_attribute(L"DefaultValue") = L"";
                    }
				}
				else if (!param.DefaultValue.empty()) {
					paramNode.append_attribute(L"DefaultValue") = param.DefaultValue;
				}
				if (!param.ChoiceList.empty()) {
					auto choiceListNode = paramNode.append_child(L"ChoiceList");
					for (const auto& item : param.ChoiceList) {
						auto itemNode = choiceListNode.append_child(L"Item");
						itemNode.append_attribute(L"Value") = item.Value;
						itemNode.text().set(item.DisplayName);
					}
				}
			}
		}
	}

    std::wostringstream oss;
	doc.save(oss);
	return str_utils::to_u16string(oss.str());
}
