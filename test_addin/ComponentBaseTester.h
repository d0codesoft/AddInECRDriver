#pragma once

#ifndef COMPONENT_BASE_TESTER_H
#define COMPONENT_BASE_TESTER_H

#include "common_platform.h"
#include <iostream>
#include <string>
#include "AddInBaseStub.h"
#include "ComponentBase.h"

class ComponentBaseTester {
public:
    ComponentBaseTester(IComponentBase* component) : component_(component) {}

    void runTests() {
        testInit();
        testSetMemManager();
        testGetInfo();
        testRegisterExtensionAs();
        tesProps();
        testMethods();
        testSetLocale();
        testSetUserInterfaceLanguageCode();
        testDone();
    }

private:
    void testInit() {
        wconsole << L"Testing Init... ";
        if (component_->Init(&addInDefBase_)) {
            wconsole << L"Success" << std::endl;
        }
        else {
            wconsole << L"Failed" << std::endl;
        }
    }

    void testSetMemManager() {
        wconsole << L"Testing setMemManager... ";
        if (component_->setMemManager(&memoryManager_)) {
            wconsole << L"Success" << std::endl;
        }
        else {
            wconsole << L"Failed" << std::endl;
        }
    }

    void testGetInfo() {
        wconsole << L"Testing GetInfo... ";
        long info = component_->GetInfo();
        wconsole << L"Info: " << info << std::endl;
    }

    void testDone() {
        wconsole << L"Testing Done... ";
        component_->Done();
        wconsole << L"Done called" << std::endl;
    }

    void testRegisterExtensionAs() {
        wconsole << L"Testing RegisterExtensionAs... ";
        WCHAR_T* extensionName = nullptr;
        if (component_->RegisterExtensionAs(&extensionName)) {
            wconsole << L"Success, Extension Name: " << toWString(extensionName) << std::endl;
        }
        else {
            wconsole << L"Failed" << std::endl;
        }
    }

    void tesProps() {
        wconsole << L"Testing GetNProps... ";
        long nProps = component_->GetNProps();
        wconsole << L"Number of Properties: " << nProps << std::endl;

        for (int nPropIndex = 0; nPropIndex < nProps; nPropIndex++) {
            wconsole << L"Property[" << nPropIndex << L"] = ";
			auto propNameEn = component_->GetPropName(nPropIndex, 0);
            auto propNameRu = component_->GetPropName(nPropIndex, 1);
            auto propIndex = component_->FindProp(propNameRu);
            auto isPropReadable = component_->IsPropReadable(propIndex);
            auto isPropWritable = component_->IsPropWritable(propIndex);
            wconsole << toWString(propNameEn) << L" / " << toWString(propNameRu) << std::endl;
			if (propIndex == -1) {
				wconsole << L"Fail - Property not found" << std::endl;
            }
            else if (propIndex != nPropIndex) {
				wconsole << L"Fail - Property index mismatch: " << propIndex << std::endl;
            }
			wconsole << L"  Readable: " << isPropReadable << L"  Writable: " << isPropWritable << std::endl;
            tVariant var;
			auto getPropVal = component_->GetPropVal(propIndex, &var);
            auto setPropVal = component_->SetPropVal(propIndex, &var);
            wconsole << L"  Get value: '" << getPropVal << L"' Set value: '" << setPropVal << L"'" << std::endl;
		}
    }

    void testMethods() {
        try {
            wconsole << L"Testing Methods... " << std::endl;
            long nMethods = component_->GetNMethods();
            wconsole << L"Number of Methods: " << nMethods << std::endl;

            for (int nMethodIndex = 0; nMethodIndex < nMethods; nMethodIndex++) {
                auto nameMethodEn = component_->GetMethodName(nMethodIndex, 0);
                auto nameMethodRu = component_->GetMethodName(nMethodIndex, 1);
                auto methodIndex = component_->FindMethod(nameMethodRu);
                auto hasRetValue = component_->HasRetVal(methodIndex);
				if (methodIndex == -1) {
					wconsole << L"Fail - Method not found" << std::endl;
				}
				else if (methodIndex != nMethodIndex) {
					wconsole << L"Fail - Method index mismatch: " << methodIndex << std::endl;
				}

                wconsole << L"Method[" << nMethodIndex << L"] : ";
                wconsole << toWString(nameMethodEn) << L" / " << toWString(nameMethodRu) << std::endl;
                long nParams = component_->GetNParams(methodIndex);
                wconsole << L"  Return value:" << hasRetValue << L"  Count parameters: " << nParams << std::endl;
                for (int nParamIndex = 0; nParamIndex < nParams; nParamIndex++) {
                    tVariant var;
                    if (component_->GetParamDefValue(0, 0, &var)) {
                        wconsole << L"  def[" << nParamIndex << L"]= '" << getVariantValue(var) << L"'" << std::endl;
                    }
                }
            }
		}
        catch (const std::exception& e) {
            wconsole << L"Error test Method: " << e.what() << std::endl;
        }
    }

    void testCallAsProc(long numProc, std::vector<tVariant>& params) {
        auto nameMethodEn = component_->GetMethodName(numProc, 0);
        auto nameMethodRu = component_->GetMethodName(numProc, 1);
        wconsole << L"Testing CallAsProc... " << toWString(nameMethodEn) << L" / " << toWString(nameMethodRu);
        if (component_->CallAsProc(numProc, params.data(), params.size())) {
            wconsole << L"Success" << std::endl;
        }
        else {
            wconsole << L"Failed" << std::endl;
        }
    }

    void testCallAsProc(std::u16string nameProc, std::vector<tVariant>& params) {
        wconsole << L"Testing CallAsProc... " << toWString(nameProc);
		long numProc = component_->FindMethod(nameProc.c_str());
        if (numProc>-1 && component_->CallAsProc(numProc, params.data(), params.size())) {
            wconsole << L"Success" << std::endl;
        }
        else {
            wconsole << L"Failed" << std::endl;
        }
    }


    void testCallAsFunc(long numFunc, std::vector<tVariant>& params, tVariant& retValue) {
        auto nameMethodEn = component_->GetMethodName(numFunc, 0);
        auto nameMethodRu = component_->GetMethodName(numFunc, 1);
        wconsole << L"Testing CallAsFunc... " << toWString(nameMethodEn) << " / " << toWString(nameMethodRu) << std::endl;
        if (component_->CallAsFunc(0, &retValue, params.data(), params.size())) {
            wconsole << L"Success" << std::endl;
            for (size_t index = 0; index < params.size(); ++index) {
                tVariant& var = params[index];
                wconsole << L"Param[" << index << L"]: " << getVariantValue(var) << std::endl;
            }

        }
        else {
            wconsole << L"Failed" << std::endl;
        }
    }

    void testSetLocale() {
        wconsole << L"Testing SetLocale... ";
        component_->SetLocale(u"en_US");
        wconsole << L"Locale set to en_US" << std::endl;
    }

    void testSetUserInterfaceLanguageCode() {
        wconsole << L"Testing SetUserInterfaceLanguageCode... ";
        component_->SetUserInterfaceLanguageCode(u"en");
        wconsole << L"User Interface Language Code set to en" << std::endl;
    }

    IComponentBase* component_;
	AddInDefBaseStub addInDefBase_;
	MemoryManagerStub memoryManager_;
};

#endif // COMPONENT_BASE_TESTER_H