// test_addin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <Windows.h>

#include "common_platform.h"
#include <iostream>
#include "AddInBaseStub.h"
#include "ComponentBase.h"
#include "ComponentBaseTester.h"
#include "wide_console.h"

int main()
{

#if defined(_WIN32)
#endif

	std::wstring libName = getLibraryName();
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

	if (!getClassObjectPtr || !destroyObjectPtr || !getClassNamesPtr || !setPlatformCapabilitiesPtr || !getAttachTypePtr) {
		wconsole << L"Error test library" << std::endl;
		return 1;
	}
	else {
		wconsole << L"Successfully found all required functions in the library." << std::endl;
	}
	
	auto class_names = getClassNamesPtr();
	auto names = splitString(class_names, u'|');
	if (names.empty()) {
		wconsole << L"Failed to get class names!" << std::endl;
		return 1;
	}

	std::wcout << L"Class names in library" << std::endl;
	for (const auto& name : names) {
		wconsole << L"	class: " << toWString(name) << std::endl;
	}
	wconsole << L"Success get class names" << std::endl;

	AppCapabilities capabilities = setPlatformCapabilitiesPtr(eAppCapabilities3);
	wconsole << L"Success set capabilities: " << capabilities << std::endl;

	AttachType attachType = getAttachTypePtr();
	wconsole << L"Success get attach type: " << attachType << std::endl;

	IComponentBase* pComponent = nullptr;
	intptr_t result = getClassObjectPtr(names[0].c_str(), &pComponent);
	if (result == 0) {
		wconsole << L"Failed to create the object!" << std::endl;
		return 1;
	}

	ComponentBaseTester tester(pComponent);
	tester.runTests();

	destroyObjectPtr(&pComponent);

	library.release();

	wconsole << L"Finish test" << std::endl;
	std::wcin.get();
	return 1;
}