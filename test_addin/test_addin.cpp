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

void displayVersionInfo() {
	wconsole << L"Test AddIn for 1C 8.X Version 1.0.0" << std::endl;
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
		return 1;
	}
	
	std::optional<std::wstring> deviceTypeDescription = std::nullopt;
	std::wstring libName = str_utils::to_wstring(std::string(argv[1]));
	if (argc == 4 && std::string(argv[2]) == "--deviceType") {
		std::wstring deviceType = str_utils::to_wstring(std::string(argv[3]));
		deviceTypeDescription = getDeviceTypeDescription(deviceType);
		if (!deviceTypeDescription.has_value()) {
			wconsole << L"Unknown device type: " << deviceType << std::endl;
			return 1;
		}
	}
	else if (argc > 2) {
		wconsole << L"Unknown option: " << str_utils::to_wstring(argv[2]) << std::endl;
		return 1;
	}

	if (!deviceTypeDescription.has_value()){
		deviceTypeDescription = getDeviceTypeDescription(L"DriverBase");
	}

	wconsole << L"Device type: " << deviceTypeDescription.value() << std::endl;
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
	tester.registerExtension<ExtensionTestAddInDriverBase>();
	tester.runTests();


	destroyObjectPtr(&pComponent);

	library.release();

	wconsole << L"Finish test" << std::endl;
	std::wcin.get();
	return 1;
}