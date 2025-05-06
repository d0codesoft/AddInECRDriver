#pragma once

#ifndef EXTENSION_TEST_SCRIPT_H
#define EXTENSION_TEST_SCRIPT_H

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <optional>
#include <string>
#include "ComponentBaseTester.h"
#include "str_utils.h"
#include "parser_call.h"

class IExtensionTestScript :
    public IExtentionTestAddIn
{
public:
    IExtensionTestScript(ITestAddIn* extTest, std::wstring fileScript)
        : IExtentionTestAddIn(extTest), fileScript(fileScript) {
    }

    bool runTest() override {
        if (fileScript.empty() || !std::filesystem::exists(fileScript)) {
            wconsole << L"Error: fileScript is empty or does not exist" << std::endl;
            return false;
        }

        bool resultTest = true;
        wconsole << L"Start testing AddIn Driver script file " << fileScript << std::endl;
        resultTest = resultTest && readScriptFile();
        wconsole << L"End testing AddIn Driver script file " << fileScript << std::endl;
        return resultTest;
    }

private:

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

	void executeLastError() {
        try {
            auto method = this->extTest_->getMethodsInfo()->findMethodByName(u"GetLastError");
            if (!method.has_value()) {
                wconsole << L"  Error: function not found: GetLastError " << std::endl;
                return;
            }

            if (method.value().countParams != 1) {
                wconsole << L"  Error: Invalid count of parameters: GetLastError" << std::endl;
                return;
            }

			std::vector<tVariant> params;
			params.insert(params.end(), method.value().countParams, tVariant());
			params[0].vt = VTYPE_EMPTY;
            tVariant retValue;
            this->extTest_->testCallAsFunc(u"GetLastError", params, retValue);
            if (TV_BOOL(&retValue)) {
				std::wstring errorDescription = getStringValue(params[0]);
				wconsole << L"  Error: " << errorDescription << std::endl;
            }
            else {
                wconsole << L"  Error execute function: GetLastError " << std::endl;
            }
        }
        catch (const std::exception& e) {
            wconsole << L"  Error execute procedure: GetLastError " << L" : " << e.what() << std::endl;
        }
	}

	bool executeCommand(const std::string& line) {

		auto call_param = parseCallString(line);
		if (!call_param.has_value()) {
			wconsole << L"Error: Invalid format in test script: " << line << std::endl;
			return false;
		}

		auto parsedCall = call_param.value();

        std::u16string execName = str_utils::to_u16string(parsedCall.methodName);
        int countParam = parsedCall.paramCount;
        std::vector<tVariant> params{};
        if (countParam > 0) {
            params = parseParams(countParam, std::span<const std::string>(parsedCall.params));
        }

        std::string resultNameValue{};
		if (call_param->resultVar.has_value()) {
            resultNameValue = call_param->resultVar.value();
		}

        if (!execName.empty()) {
            try {
				auto method = this->extTest_->getMethodsInfo()->findMethodByName(execName);
                if (!method.has_value()) {
                    wconsole << L"  Error: Procedure not found: " << str_utils::to_wstring(execName) << std::endl;
                    return false;
                }

                if (method.value().countParams != params.size()) {
					wconsole << L"  Error: Invalid count of parameters: " << str_utils::to_wstring(execName) << std::endl;
					return false;
                }

                if (parsedCall.callType == CallType::CallAsFunc) {
                    tVariant retValue;
					this->extTest_->testCallAsFunc(execName, params, retValue);

                    if (!resultNameValue.empty()) {
						_saveResultValues(resultNameValue, params, retValue);
                    }

					if (TV_BOOL(&retValue) && retValue.bVal) {
						return true;
					}
                    else {
                        wconsole << L"  Error execute function: " << str_utils::to_wstring(execName) << std::endl;
                        return false;
                    }
				}
                else if (parsedCall.callType == CallType::CallAsProc) {
                    this->extTest_->testCallAsProc(execName, params);
                    if (!resultNameValue.empty()) {
                        _saveValues(resultNameValue, params);
                    }
                }
				return true;
			}
			catch (const std::exception& e) {
				wconsole << L"  Error execute procedure: " << str_utils::to_wstring(execName) << L" : " << e.what() << std::endl;
			}
        }
        return false;
    }

    std::vector<tVariant> parseParams(const int countParams, const std::span<const std::string>& tokens) {
        std::vector<tVariant> params(countParams);
		int i = 0;
        for (auto& var : tokens) {
			if (i >= countParams) break;
			setVariantValue(params[i], var);
			i++;
        }
        return params;
    }

    bool setVariantValue(tVariant& var, const std::string& str) {
        if (str.empty()) {
            var.vt = VTYPE_EMPTY;
            return true;
        }

        if (str.find("[") != std::string::npos && str.find("]") != std::string::npos) {
			// Если строка содержит квадратные скобки, интерпретируем как переменную хранящую данные предыдущих вызовов
			std::string nameContent = str.substr(0, str.find("[")); // Удаляем квадратные скобки
			std::string content = str.substr(str.find("[") + 1, str.find("]") - str.find("[") - 1); // Удаляем квадратные скобки
			auto it = paramsExecute.find(str_utils::to_wstring(nameContent));
			if (it != paramsExecute.end()) {
                int indexValue = -1;
                try {
                    indexValue = std::stoi(content);
                }
                catch (...) {
                    var.vt = VTYPE_EMPTY;
                    wconsole << L"  Error: Set variant value Invalid convert index value: " << str_utils::to_wstring(nameContent) << L" index: " << str_utils::to_wstring(content) << std::endl;
                    return false;
                }
                
                if (it->second.empty() && it->second.size()<indexValue) {
					var.vt = VTYPE_EMPTY;
					wconsole << L"  Error: index variable: " << str_utils::to_wstring(nameContent) << std::endl;
					return false;
				}

				auto _val = it->second[indexValue];
				if (std::holds_alternative<std::wstring>(_val)) {
					std::wstring value = std::get<std::wstring>(_val);
					var.vt = VTYPE_PWSTR;
					this->extTest_->getMemoryManager()->AllocMemory(
						reinterpret_cast<void**>(&var.pwstrVal),
						(static_cast<const long>(value.size()) + 1) * sizeof(char16_t)
					);
					std::copy(value.begin(), value.end(), reinterpret_cast<char16_t*>(var.pwstrVal));
					var.pwstrVal[value.size()] = u'\0';
				}
				else if (std::holds_alternative<long>(_val)) {
					var.vt = VTYPE_I4;
					var.intVal = std::get<long>(_val);
				}
				else if (std::holds_alternative<double>(_val)) {
					var.vt = VTYPE_R8;
					var.dblVal = std::get<double>(_val);
				}
				else if (std::holds_alternative<bool>(_val)) {
					var.vt = VTYPE_BOOL;
					var.bVal = std::get<bool>(_val);
				}
				else {
					var.vt = VTYPE_EMPTY;
					wconsole << L"  Error: Set variant value Unsupported type for parameter: " << str_utils::to_wstring(nameContent) << std::endl;
					return false;
				}
				return true;
			}
			return true;
        }

        // Если строка в кавычках, интерпретируем как строку
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            std::string content = str.substr(1, str.size() - 2); // Удаляем кавычки
            std::u16string val = str_utils::to_u16string(content);
            var.vt = VTYPE_PWSTR;
            this->extTest_->getMemoryManager()->AllocMemory(
                reinterpret_cast<void**>(&var.pwstrVal),
                (static_cast<const long>(val.size()) + 1) * sizeof(char16_t)
            );
            std::copy(val.begin(), val.end(), reinterpret_cast<char16_t*>(var.pwstrVal));
            var.pwstrVal[val.size()] = u'\0';
            return true;
        }
        else if (str == "true" || str == "false") {
            var.vt = VTYPE_BOOL;
            var.bVal = (str == "true");
            return true;
        }
        else if (str.find('.') != std::string::npos) {
            try {
                var.vt = VTYPE_R8;
                var.dblVal = std::stod(str);
                return true;
            }
            catch (...) {}
        }
        else if (std::all_of(str.begin(), str.end(), ::isdigit)) {
            try {
                var.vt = VTYPE_I4;
                var.intVal = std::stoi(str);
                return true;
            }
            catch (...) {}
        }

        // По умолчанию — строка
        std::u16string val = str_utils::to_u16string(str);
        var.vt = VTYPE_PWSTR;
        this->extTest_->getMemoryManager()->AllocMemory(
            reinterpret_cast<void**>(&var.pwstrVal),
            (static_cast<const long>(val.size()) + 1) * sizeof(char16_t)
        );
        std::copy(val.begin(), val.end(), reinterpret_cast<char16_t*>(var.pwstrVal));
        var.pwstrVal[val.size()] = u'\0';

        return true;
    }

    void startTest(const std::string& line) {
        // Удаляем пробелы из строки
        std::string trimmedLine = trim(line);

        // Разделяем строку по символу "="
        size_t delimiterPos = trimmedLine.find('=');
        if (delimiterPos == std::string::npos) {
            wconsole << L"Error: Invalid format in [Test] section: " << str_utils::to_wstring(line) << std::endl;
            return;
        }

        // Извлекаем ключ (первый параметр) и значение (второй параметр)
        std::string key = trimmedLine.substr(0, delimiterPos);
        std::string value = trimmedLine.substr(delimiterPos + 1);

        // Удаляем пробелы из ключа и значения
        key = trim(key);
        value = trim(value);

        // Проверяем, что ключ равен "Name"
        if (key != "Name") {
            wconsole << L"Error: Unexpected key in [Test] section: " << str_utils::to_wstring(key) << std::endl;
            return;
        }

        // Убираем кавычки из значения, если они есть
        if (!value.empty() && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        // Печатаем секции "Start test" и "Name"
        wconsole << L"---------------------------------------------" << std::endl;
        wconsole << L"Start test section " << str_utils::to_wstring(value) << std::endl;
    }

	bool readScriptFile() {
        std::ifstream file(fileScript);
        if (!file.is_open()) {
            wconsole << L"  Error: Cannot open test script file: " << fileScript << std::endl;
            return false;
        }

        std::string line;
        std::string section;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            if (line[0] == '[' && line.back() == ']') {
                section = line.substr(1, line.size() - 2);
                continue;
            }

            if (section == "Execute") {
                if (!executeCommand(line)) {
                    executeLastError();
                }
            }
            else if (section == "Test") {
                startTest(line);
            }
        }

        return true;
	}

    void _saveResultValues(std::string nameResultValue, std::vector<tVariant>& params, tVariant& result) {
		std::wstring _name = str_utils::to_wstring(nameResultValue);
		paramsExecute[_name].clear();
        int iCount = 0;
        for (const auto& param : params) {
            if (param.vt == VTYPE_EMPTY) {
				paramsExecute[_name].emplace_back(std::monostate{});
				wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << L"Empty" << std::endl;
            }
            else if (param.vt == VTYPE_PWSTR || param.vt == VTYPE_PSTR) {
                std::wstring value = getStringValue(param);
                paramsExecute[_name].emplace_back(value);
				wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << value << std::endl;
            }
            else if (param.vt == VTYPE_I4) {
                paramsExecute[_name].emplace_back(static_cast<long>(param.lVal));
                wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << param.lVal << std::endl;
            }
            else if (param.vt == VTYPE_I2) {
				paramsExecute[_name].emplace_back(static_cast<long>(param.shortVal));
				wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << param.shortVal << std::endl;
            }
			else if (param.vt == VTYPE_INT) {
				paramsExecute[_name].emplace_back(static_cast<long>(param.intVal));
				wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << param.intVal << std::endl;
			}
			else if (param.vt == VTYPE_UINT) {
				paramsExecute[_name].emplace_back(static_cast<long>(param.uintVal));
				wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << param.uintVal << std::endl;
			}
            else if (param.vt == VTYPE_R8) {
                paramsExecute[_name].emplace_back(param.dblVal);
                wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << param.dblVal << std::endl;
            }
            else if (param.vt == VTYPE_BOOL) {
                paramsExecute[_name].emplace_back(param.bVal);
                wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << param.bVal << std::endl;
            }
            else {
                paramsExecute[_name].emplace_back(std::monostate{});
                wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = Error: Unsupported type for parameter " << std::endl;
            }
            iCount++;
        }

        //result 
		if (result.vt == VTYPE_EMPTY) {
			paramsExecute[_name].emplace_back(std::monostate{});
			wconsole << L"  " << _name << L"[" << iCount << L"]" << L" = " << L"Empty" << std::endl;
		}
        else  if (result.vt == VTYPE_PWSTR || result.vt == VTYPE_PSTR) {
            std::wstring value = getStringValue(result);
            paramsExecute[_name].emplace_back(value);
            wconsole << L"  R " << _name << L"[" << iCount << L"]" << L" = " << value << std::endl;
        }
        else if (result.vt == VTYPE_I4) {
            paramsExecute[_name].emplace_back(result.intVal);
            wconsole << L"  R " << _name << L"[" << iCount << L"]" << L" = " << result.dblVal << std::endl;
        }
        else if (result.vt == VTYPE_R8) {
            paramsExecute[_name].emplace_back(result.dblVal);
            wconsole << L"  R " << _name << L"[" << iCount << L"]" << L" = " << result.dblVal << std::endl;
        }
        else if (result.vt == VTYPE_BOOL) {
            paramsExecute[_name].emplace_back(result.bVal);
            wconsole << L"  R " << _name << L"[" << iCount << L"]" << L" = " << result.bVal << std::endl;
        }
        else {
            paramsExecute[_name].emplace_back(std::monostate{});
            wconsole << L"  R " << _name << L"[" << iCount << L"]" << L" = Error: Unsupported type for parameter " << std::endl;
        }
    }

    void _saveValues(std::string nameResultValue, std::vector<tVariant>& params) {
        std::wstring _name = str_utils::to_wstring(nameResultValue);
        paramsExecute[_name].clear();
        for (const auto& param : params) {
			if (param.vt == VTYPE_EMPTY) {
				paramsExecute[_name].emplace_back(std::monostate{});
			}
			else if (param.vt == VTYPE_PWSTR || param.vt == VTYPE_PSTR) {
                std::wstring value = getStringValue(param);
                paramsExecute[_name].emplace_back(value);
            }
            else if (param.vt == VTYPE_I4) {
                paramsExecute[_name].emplace_back(param.intVal);
            }
            else if (param.vt == VTYPE_R8) {
                paramsExecute[_name].emplace_back(param.dblVal);
            }
            else if (param.vt == VTYPE_BOOL) {
                paramsExecute[_name].emplace_back(param.bVal);
            }
            else {
                paramsExecute[_name].emplace_back(std::monostate{});
                wconsole << L"  Error: Unsupported type for parameter: " << _name << std::endl;
            }
        }
    }

	std::wstring fileScript;
	std::unordered_map<std::wstring, std::vector<std::variant<std::wstring, long, double, bool, std::monostate>>> paramsExecute;
};

#endif // EXTENSION_TEST_SCRIPT_H

