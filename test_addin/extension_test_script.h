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

	bool executeCommand(const std::string& line) {

		auto call_param = parseCallString(line);
		if (!call_param.has_value()) {
			wconsole << L"Error: Invalid format in test script: " << line << std::endl;
			return false;
		}

		auto parsedCall = call_param.value();

        std::u16string execName = str_utils::to_u16string(parsedCall.methodName);
        int countParam = parsedCall.paramCount;
		std::vector<tVariant> params = parseParams(countParam, std::span<const std::string>(parsedCall.params));

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

                if (parsedCall.callType== CallType::CallAsFunc) {
                    tVariant retValue;
					this->extTest_->testCallAsFunc(execName, params, retValue);
					if (TV_BOOL(&retValue)) {
						return true;
					}
                    else {
                        wconsole << L"  Error execute function: " << str_utils::to_wstring(execName) << std::endl;
                    }
				}
                else if (parsedCall.callType == CallType::CallAsProc) {
                    this->extTest_->testCallAsProc(execName, params);
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
        if (str == "true" || str == "false") {
            var.vt = VTYPE_BOOL;
            var.bVal = (str == "true");
            return true;
        }
        else if (str.find('.') != std::string::npos) {
            var.vt = VTYPE_R8;
            var.dblVal = std::stod(str);
            return true;
        }
        else if (std::all_of(str.begin(), str.end(), ::isdigit)) {
            var.vt = VTYPE_I4;
            var.intVal = std::stoi(str);
            return true;
        }
		else if (str.empty()) {
			var.vt = VTYPE_EMPTY;
			return true;
		}
        else {
			std::u16string val = str_utils::to_u16string(str);
            var.vt = VTYPE_PWSTR;
			this->extTest_->getMemoryManager()->AllocMemory(reinterpret_cast<void**>(&var.pwstrVal), (str.size() + 1) * sizeof(char16_t));
            std::copy(val.begin(), val.end(), reinterpret_cast<char16_t*>(var.pwstrVal));
            var.pwstrVal[str.size()] = u'\0';
            return true;
        }
		return false;
    }

    bool executeFunction(const std::string& line) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (std::getline(iss, token, '|')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() < 2) {
            wconsole << L"Error: Invalid procedure format in test script: " << line << std::endl;
            return false;
        }

        std::u16string funcName = str_utils::to_u16string(tokens[0]);
        int countParam = std::stoi(tokens[1]);
        std::vector<tVariant> params = parseParams(countParam, std::span<const std::string>(tokens).subspan(2));
		tVariant retValue;

        if (!funcName.empty()) {
            try {
                auto method = this->extTest_->getMethodsInfo()->findMethodByName(funcName);
                if (!method.has_value()) {
                    wconsole << L"  Error: Procedure not found: " << str_utils::to_wstring(funcName) << std::endl;
                    return false;
                }


            }
            catch (const std::exception& e) {
                wconsole << L"  Error execute function : " << str_utils::to_wstring(funcName) << L" : " << e.what() << std::endl;
            }
        }
        return false;
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
                executeCommand(line);
            }
        }

        return true;
	}


	std::wstring fileScript;
};

#endif // EXTENSION_TEST_SCRIPT_H

