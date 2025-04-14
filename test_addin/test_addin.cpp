// test_addin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <Windows.h>

#include "common_platform.h"
#include <iostream>
#include "AddInBaseStub.h"
#include "ComponentBase.h"
#include "ComponentBaseTester.h"
#include "wide_console.h"
#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include "extension_test_script.h"

void displayVersionInfo() {
	wconsole << L"Test AddIn for 1C 8.X Version 1.0.0" << std::endl;
}

std::optional<std::wstring> findParam(
	const std::unordered_map<std::wstring, std::wstring>& options,
	const std::wstring& name)
{
	auto it = options.find(name);
	if (it != options.end()) {
		return it->second; // Return the found value
	}
	return std::nullopt; // Parameter not found
}

std::unordered_map<std::wstring, std::wstring> parseArgs(int argc, char* argv[]) {
	std::unordered_map<std::wstring, std::wstring> options;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if (arg.rfind("--", 0) == 0) { // Starts with "--"
			std::wstring key = str_utils::to_wstring(arg);
			std::wstring value = {};

			if (i + 1 < argc) {
				std::wstring nextArg = str_utils::to_wstring(argv[i + 1]);

				if (nextArg.rfind(L"--", 0) != 0) { // Not another option
					value = nextArg;
					++i; // Skip next arg
				}
			}
			options[key] = value;
		}
	}
	return options;
}


int main(int argc, char* argv[])
{
	displayVersionInfo();

	if (argc < 2) {
		wconsole << L"Usage: test_addin <DLL_NAME>" << std::endl;
		wconsole << L"(Optional): " << std::endl;
		wconsole << L"	--deviceType <Type device AddIn> default testing DriverBase" << std::endl;
		wconsole << L"	Supported Type device AddIn:" << std::endl;
		for (const auto& device : getDeviceTypesRu()) {
			wconsole << L"		" << device.first << L" - " << device.second << std::endl;
		}
		wconsole << L"	--script <full path file script *.testscript>" << std::endl;
		wconsole << L"		see exampl pos_terminal.testscript" << std::endl;
		return 1;
	}
	
	std::wstring libName = str_utils::to_wstring(std::string(argv[1]));
	if (libName.empty()) {
		wconsole << L"Error name AddIn library is empty!" << std::endl;
		return 1;
	}
	if (!std::filesystem::exists(libName) || std::filesystem::is_directory(libName)) {
		wconsole << L"File AddIn library " << libName << " not found! Current folder: " << std::filesystem::current_path() << std::endl;
		return 1;
	}
	if (std::filesystem::path(libName).extension() != LIB_EXTENSION) {
		wconsole << L"File AddIn library not extension library!" << std::endl;
		return 1;
	}

	auto paramProgram = parseArgs(argc-1, &argv[1]);

	std::optional<std::wstring> deviceTypeDescription = std::nullopt;
	auto parDeviseType = findParam(paramProgram, L"--deviceType");
	if (parDeviseType.has_value()) {
		std::wstring deviceType = parDeviseType.value();
		deviceTypeDescription = getDeviceTypeDescription(deviceType);
		if (!deviceTypeDescription.has_value()) {
			wconsole << L"Unknown device type: " << deviceType << std::endl;
			return 1;
		}
		wconsole << L"Device type: " << deviceTypeDescription.value() << std::endl;
	}

	std::wstring fileScript = {};
	auto parFileScript = findParam(paramProgram, L"--script");
	if (parFileScript.has_value()) {
		fileScript = parFileScript.value();
		if (!std::filesystem::exists(fileScript) || std::filesystem::is_directory(fileScript)) {
			wconsole << L"File script not found!" << std::endl;
			return 1;
		}
	}

	auto library = std::make_unique<SharedLibrary>(libName);
    if (!library->isValid()) {
		wconsole << L"Failed to load the library!" << std::endl;
        return 1;
    }

    GetClassObjectPtr getClassObjectPtr = GetProcAddressSafe<GetClassObjectPtr>(library->getHandle(), "GetClassObject");
    DestroyObjectPtr destroyObjectPtr = GetProcAddressSafe<DestroyObjectPtr>(library->getHandle(), "DestroyObject");
    GetClassNamesPtr getClassNamesPtr = GetProcAddressSafe<GetClassNamesPtr>(library->getHandle(), "GetClassNames");
    SetPlatformCapabilitiesPtr setPlatformCapabilitiesPtr = GetProcAddressSafe<SetPlatformCapabilitiesPtr>(library->getHandle(), "SetPlatformCapabilities");
    GetAttachTypePtr getAttachTypePtr = GetProcAddressSafe<GetAttachTypePtr>(library->getHandle(), "GetAttachType");

	if (!getClassObjectPtr || !destroyObjectPtr || !getClassNamesPtr ) {
		wconsole << L"Error test library" << std::endl;
		return 1;
	}
	else {
		wconsole << L"Successfully found all required functions in the library." << std::endl;
	}
	
	if (!setPlatformCapabilitiesPtr) {
		wconsole << L"Error find export function SetPlatformCapabilities" << std::endl;
	}

	if (!getAttachTypePtr) {
		wconsole << L"Error find export function GetAttachType" << std::endl;
	}

	auto class_names = getClassNamesPtr();
	auto names = splitString(class_names, u'|');
	if (names.empty()) {
		wconsole << L"Failed to get class names!" << std::endl;
		return 1;
	}

	std::wcout << L"Class names in library" << std::endl;
	for (const auto& name : names) {
		wconsole << L"	class: " << str_utils::to_wstring(name) << std::endl;
	}
	wconsole << L"Success get class names" << std::endl;

	if (setPlatformCapabilitiesPtr) {
		AppCapabilities capabilities = setPlatformCapabilitiesPtr(eAppCapabilities3);
		wconsole << L"Success set capabilities: " << capabilities << std::endl;
	}

	if (getAttachTypePtr) {
		AttachType attachType = getAttachTypePtr();
		wconsole << L"Success get attach type: " << attachType << std::endl;
	}

	IComponentBase* pComponent = nullptr;
	intptr_t result = getClassObjectPtr(names[0].c_str(), &pComponent);
	if (result == 0) {
		wconsole << L"Failed to create the object!" << std::endl;
		return 1;
	}

	TestAddIn tester(pComponent);
	if (deviceTypeDescription.has_value()) {
		tester.registerExtension<ExtensionTestAddInDriverBase>();
	}
	if (!fileScript.empty()) {
		tester.registerExtension<IExtensionTestScript>(fileScript);
	}

	tester.runTests();

	destroyObjectPtr(&pComponent);

	library.release();

	wconsole << L"Finish test" << std::endl;
	std::wcin.get();
	return 1;
}