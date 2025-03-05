#pragma once

#ifndef COMPONENT_BASE_TESTER_H
#define COMPONENT_BASE_TESTER_H

#include "ComponentBase.h"
#include <iostream>
#include <string>
#include "AddInBaseStub.h"

class ComponentBaseTester {
public:
    ComponentBaseTester(IComponentBase* component) : component_(component) {}

    void runTests() {
        testInit();
        testSetMemManager();
        testGetInfo();
        testDone();
        testRegisterExtensionAs();
        tesProps();
        testMethods();
        testSetLocale();
        testSetUserInterfaceLanguageCode();
    }

private:
    void testInit() {
        std::wcout << L"Testing Init... ";
        if (component_->Init(&addInDefBase_)) {
            std::wcout << L"Success" << std::endl;
        }
        else {
            std::wcout << L"Failed" << std::endl;
        }
    }

    void testSetMemManager() {
        std::wcout << L"Testing setMemManager... ";
        if (component_->setMemManager(&memoryManager_)) {
            std::wcout << L"Success" << std::endl;
        }
        else {
            std::wcout << L"Failed" << std::endl;
        }
    }

    void testGetInfo() {
        std::wcout << L"Testing GetInfo... ";
        long info = component_->GetInfo();
        std::wcout << L"Info: " << info << std::endl;
    }

    void testDone() {
        std::wcout << L"Testing Done... ";
        component_->Done();
        std::wcout << L"Done called" << std::endl;
    }

    void testRegisterExtensionAs() {
        std::wcout << L"Testing RegisterExtensionAs... ";
        WCHAR_T* extensionName = nullptr;
        if (component_->RegisterExtensionAs(&extensionName)) {
            std::wcout << L"Success, Extension Name: " << toWString(extensionName) << std::endl;
        }
        else {
            std::wcout << L"Failed" << std::endl;
        }
    }

    void tesProps() {
        std::wcout << L"Testing GetNProps... ";
        long nProps = component_->GetNProps();
        std::wcout << L"Number of Properties: " << nProps << std::endl;

        for (int nPropIndex = 0; nPropIndex < nProps; nPropIndex++) {
			std::wcout << L"Property test index: " << nPropIndex << std::endl;
			auto propNameEn = component_->GetPropName(nPropIndex, 0);
            auto propNameRu = component_->GetPropName(nPropIndex, 1);
            auto propIndex = component_->FindProp(propNameRu);
            auto isPropReadable = component_->IsPropReadable(propIndex);
            auto isPropWritable = component_->IsPropWritable(propIndex);
            std::wcout << L"Property Name: " << toWString(propNameEn) << L" / " << toWString(propNameRu) << std::endl;
			std::wcout << L"Property Index: " << propIndex << std::endl;
			std::wcout << L"Is Property Readable: " << isPropReadable << std::endl;
			std::wcout << L"Is Property Writable: " << isPropWritable << std::endl;
			std::wcout << L"Testing Get / Set property value ... ";
            
            tVariant var;
			auto getPropVal = component_->GetPropVal(propIndex, &var);
			std::wcout << L"Get Property Value: " << getPropVal << std::endl;
			auto setPropVal = component_->SetPropVal(propIndex, &var);
			std::wcout << L"Set Property Value: " << setPropVal << std::endl;
		}
    }

    void testMethods() {
        try {
            std::wcout << L"Testing Methods... ";
            long nMethods = component_->GetNMethods();
            std::wcout << L"Number of Methods: " << nMethods << std::endl;

            for (int nMethodIndex = 0; nMethodIndex < nMethods; nMethodIndex++) {
                std::wcout << L"Method test index: " << nMethodIndex << std::endl;
                auto nameMethodEn = component_->GetMethodName(nMethodIndex, 0);
                auto nameMethodRu = component_->GetMethodName(nMethodIndex, 1);
                auto methodIndex = component_->FindMethod(nameMethodRu);
                auto hasRetValue = component_->HasRetVal(methodIndex);

                std::wcout << L"Method Name: " << toWString(nameMethodEn) << L" / " << toWString(nameMethodRu) << std::endl;
                std::wcout << L"Method Index: " << methodIndex << std::endl;
                std::wcout << L"Testing GetNParams... ";
                long nParams = component_->GetNParams(methodIndex);
                std::wcout << L"Number of Parameters: " << nParams << std::endl;
                std::wcout << L"Testing GetParamDefValue... ";
                for (int nParamIndex = 0; nParamIndex < nParams; nParamIndex++) {
                    tVariant var;
                    if (component_->GetParamDefValue(0, 0, &var)) {
                        std::wcout << L"Success" << std::endl;
                    }
                    else {
                        std::wcout << L"Not def value" << std::endl;
                    }
                }
                std::wcout << L"Has return value: " << hasRetValue << std::endl;
            }
		}
        catch (const std::exception& e) {
            std::wcout << L"Error test Method: " << e.what() << std::endl;
        }
    }

    void testCallAsProc(long numProc, std::vector<tVariant>& params) {
        auto nameMethodEn = component_->GetMethodName(numProc, 0);
        auto nameMethodRu = component_->GetMethodName(numProc, 1);
        std::wcout << L"Testing CallAsProc... " << toWString(nameMethodEn) << L" / " << toWString(nameMethodRu);
        if (component_->CallAsProc(numProc, params.data(), params.size())) {
            std::wcout << L"Success" << std::endl;
        }
        else {
            std::wcout << L"Failed" << std::endl;
        }
    }

    void testCallAsProc(std::u16string nameProc, std::vector<tVariant>& params) {
        std::wcout << L"Testing CallAsProc... " << toWString(nameProc);
		long numProc = component_->FindMethod(nameProc.c_str());
        if (numProc>-1 && component_->CallAsProc(numProc, params.data(), params.size())) {
            std::wcout << L"Success" << std::endl;
        }
        else {
            std::wcout << L"Failed" << std::endl;
        }
    }


    void testCallAsFunc(long numFunc, std::vector<tVariant>& params, tVariant& retValue) {
        auto nameMethodEn = component_->GetMethodName(numFunc, 0);
        auto nameMethodRu = component_->GetMethodName(numFunc, 1);
        std::wcout << L"Testing CallAsFunc... " << toWString(nameMethodEn) << " / " << toWString(nameMethodRu) << std::endl;
        if (component_->CallAsFunc(0, &retValue, params.data(), params.size())) {
            std::wcout << L"Success" << std::endl;
            for (size_t index = 0; index < params.size(); ++index) {
                tVariant& var = params[index];
                std::wcout << L"Param[" << index << L"]: " << getVariantValue(var) << std::endl;
            }

        }
        else {
            std::wcout << L"Failed" << std::endl;
        }
    }

    void testSetLocale() {
        std::wcout << L"Testing SetLocale... ";
        component_->SetLocale(u"en_US");
        std::wcout << L"Locale set to en_US" << std::endl;
    }

    void testSetUserInterfaceLanguageCode() {
        std::wcout << L"Testing SetUserInterfaceLanguageCode... ";
        component_->SetUserInterfaceLanguageCode(u"en");
        std::wcout << L"User Interface Language Code set to en" << std::endl;
    }

    IComponentBase* component_;
	AddInDefBaseStub addInDefBase_;
	MemoryManagerStub memoryManager_;
};

#endif // COMPONENT_BASE_TESTER_H