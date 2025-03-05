// test_addin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <Windows.h>

#include "common_platform.h"
#include <iostream>
#include "AddInBaseStub.h"
#include "ComponentBase.h"

int main()
{
	std::wstring libName = getLibraryName();
	auto library = std::make_unique<SharedLibrary>(libName);
    if (!library->isValid()) {
        std::cerr << "Failed to load the library!" << std::endl;
        return 1;
    }

    GetClassObjectPtr getClassObjectPtr = GetProcAddressSafe<GetClassObjectPtr>(library->getHandle(), "GetClassObject");
    DestroyObjectPtr destroyObjectPtr = GetProcAddressSafe<DestroyObjectPtr>(library->getHandle(), "DestroyObject");
    GetClassNamesPtr getClassNamesPtr = GetProcAddressSafe<GetClassNamesPtr>(library->getHandle(), "GetClassNames");
    SetPlatformCapabilitiesPtr setPlatformCapabilitiesPtr = GetProcAddressSafe<SetPlatformCapabilitiesPtr>(library->getHandle(), "SetPlatformCapabilities");
    GetAttachTypePtr getAttachTypePtr = GetProcAddressSafe<GetAttachTypePtr>(library->getHandle(), "GetAttachType");

	if (!getClassObjectPtr || !destroyObjectPtr || !getClassNamesPtr || !setPlatformCapabilitiesPtr || !getAttachTypePtr) {
		std::cerr << "Error test library" << std::endl;
		return 1;
	}
	else {
		std::wcout << L"Successfully found all required functions in the library." << std::endl;
	}
	
	auto class_names = getClassNamesPtr();
	auto names = splitString(class_names, u'|');
	if (names.empty()) {
		std::cerr << "Failed to get class names!" << std::endl;
		return 1;
	}

	std::wcout << L"Class names in library" << std::endl;
	for (const auto& name : names) {
		std::wcout << L"	class: " << toWString(name) << std::endl;
	}
	std::wcout << L"Success get class names" << std::endl;

	AppCapabilities capabilities = setPlatformCapabilitiesPtr(eAppCapabilities3);
	std::wcout << L"Success set capabilities: " << capabilities << std::endl;

	AttachType attachType = getAttachTypePtr();
	std::wcout << L"Success get attach type: " << attachType << std::endl;

	IComponentBase* pComponent = nullptr;
	intptr_t result = getClassObjectPtr(names[0].c_str(), &pComponent);
	if (result == 0) {
		std::cerr << "Failed to create the object!" << std::endl;
		return 1;
	}

	

	destroyObjectPtr(&pComponent);

	library.release();

    std::cout << "Finish test\n";
}