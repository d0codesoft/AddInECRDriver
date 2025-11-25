#include "pch.h"
#include "setting_driver_pos.h"
#include <pugixml.hpp>
#include <sstream>
#include "string_conversion.h"
#include "str_utils.h"
#include "logger.h"
#include "connection_types.h"

SettingDriverPos::SettingDriverPos()
    : m_settings({
    {
        L"Параметри", // PageCaption
        {
            {   // Group Connection parameters
                L"Параметри підключення",
                {
                    { OptionDriverNames.at(DriverOption::ConnectionType), 
                        L"Тип підключення", L"Виберіть тип підключення", L"Number", L"", L"0", false,
                     { 
                         { getConnectionTypeIndex(ConnectionType::TCP), getConnectionTypeName(ConnectionType::TCP) }, 
                         { getConnectionTypeIndex(ConnectionType::COM), getConnectionTypeName(ConnectionType::COM) }, 
                         { getConnectionTypeIndex(ConnectionType::WebSocket), getConnectionTypeName(ConnectionType::WebSocket) }
                      }
                    },
                    { OptionDriverNames.at(DriverOption::Address), 
                        L"Адреса підключення", L"Введіть адресу сервера", L"String", L"", L"", false, {}
                    },
                    { OptionDriverNames.at(DriverOption::Port), 
                        L"Порт", L"Введіть номер порту", L"Number", L"", L"2000", false, {}
                    },
                    { OptionDriverNames.at(DriverOption::ResponseTimeout),
                        L"Час очікування відповіді (сек.)", L"Введіть кількіть секунд", L"Number", L"", L"30", false, {}
                    }
                    //{L"Speed", L"Скорость подключения", L"Укажите скорость соединения.", L"Number", L"", L"9600", false, {}}
                }
            },
			{   // Group payement parameters
				L"Параметри оплати",
				{
					{ OptionDriverNames.at(DriverOption::Facepay), 
                        L"Дозволити оплату через FacePay24", L"", L"Boolean", L"", L"false", false,{}},
					{ OptionDriverNames.at(DriverOption::MerchantId), 
                        L"Код мерчанта", L"Код мерчанта", L"Number", L"", L"", false,{}},
                    { OptionDriverNames.at(DriverOption::PrintReceiptOnTerminal),
                        L"Друкувати чек на терміналі", L"Друкувати чек на терміналі", L"Boolean", L"", L"false", false,{}},
                }
			},
			{   // Logging parameters
				L"Логування",
				{
					{ OptionDriverNames.at(DriverOption::LogFullPath), 
                        L"Поточний файл лога", L"", L"String", L"", L"", true, {}
                    },
					{ OptionDriverNames.at(DriverOption::LogLevel), 
                        L"Рівень логування", L"Виберіть рівень логування", L"Number", L"", L"0", false,
					    {
                         { getLogLevelIndex(LogLevel::Error), getLogLevelName(LogLevel::Error) },
                         { getLogLevelIndex(LogLevel::Debug), getLogLevelName(LogLevel::Debug) }
                        }
                    },
				}
			},
            {   // Group licensing
                L"Ліцензування",
                {
                    {L"DriverVersion", L"Версія драйвера", L"", L"String", L"", L"1.0.0", true, {}},
                    {L"LicenseStatus", L"Статус ліцензії", L"", L"String", L"", L"Не активована", true, {}},
                    {L"LicenseKey", L"Ключ ліцензії", L"Введіть ключ ліцензії", L"String", L"", L"", false, {} }
                }
            }
        }
    }
    })
{
}

const SettingSettings& SettingDriverPos::getSettings()
{
	return m_settings;
}

std::u16string SettingDriverPos::getSettingXML()
{
    pugi::xml_document doc;
    //auto decl = doc.append_child(pugi::node_declaration);
    //decl.append_attribute(L"version") = L"1.0";
    //decl.append_attribute(L"encoding") = L"UTF-8";

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
	//auto decl = doc.append_child(pugi::node_declaration);
	//decl.append_attribute(L"version") = L"1.0";
	//decl.append_attribute(L"encoding") = L"UTF-8";

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
						paramNode.append_attribute(L"DefaultValue") = param.DefaultValue;
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
