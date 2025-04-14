#pragma once

#ifndef COMPONENT_BASE_TESTER_H
#define COMPONENT_BASE_TESTER_H

#include "common_platform.h"
#include <iostream>
#include <string>
#include "AddInBaseStub.h"
#include "ComponentBase.h"
#include "str_utils.h"
#include <regex>
#include <span>

#define FREE_TVARIANT_MEMORY(memoryManager, variant) \
    do { \
        if (TV_VT(&(variant)) == VTYPE_PWSTR) { \
            (memoryManager).FreeMemory(reinterpret_cast<void**>(&(variant).pwstrVal)); \
        } else if (TV_VT(&(variant)) == VTYPE_PSTR) { \
            (memoryManager).FreeMemory(reinterpret_cast<void**>(&(variant).pstrVal)); \
        } \
    } while (0)

#define FREE_MEMORY_CONST(memoryManager, ptr) \
    (memoryManager).FreeMemory(reinterpret_cast<void**>(const_cast<WCHAR_T**>(&(ptr))))

#define INIT_TVARIANT_VECTOR(vec, count) \
    std::vector<tVariant> vec(count); \
    for (auto& var : vec) { \
        TV_VT(&var) = VTYPE_EMPTY; \
    }

std::optional<std::wstring> getDeviceTypeDescription(const std::wstring & deviceType);
std::unordered_map<std::wstring, std::wstring>& getDeviceTypesEn();
std::unordered_map<std::wstring, std::wstring>& getDeviceTypesRu();
std::wstring getStringValue(const tVariant & var);
long getLongValue(const tVariant & var);
std::optional<bool> getBoolValue(const tVariant & var);
bool isValueString(const tVariant & var);

class ITestAddIn;

class IExtentionTestAddIn {
public:
    IExtentionTestAddIn(ITestAddIn* extTest) : extTest_(extTest) {}

    virtual ~IExtentionTestAddIn() = default;

    virtual bool runTest() = 0;

protected:

    ITestAddIn* extTest_;
};

struct MethodInfo {
	std::u16string nameEn;
    std::u16string nameRu;
	int indexMethod;
	bool hasReturnValue;
	int countParams;
};

class MethodManager {
private:
    std::vector<MethodInfo> methodsNames_;

public:
    std::span<const MethodInfo> getMethodsInfo() const {
        return std::span<const MethodInfo>(methodsNames_);
    }

    void addMethod(const std::u16string& nameEn, const std::u16string& nameRu, int index) {
        methodsNames_.push_back({ nameEn, nameRu, index });
    }

	void addMethod(MethodInfo method) {
		methodsNames_.push_back(method);
	}

    std::optional<MethodInfo> findMethodByName(const std::u16string& name) const {
        for (const auto& method : getMethodsInfo()) {
            if (method.nameEn == name || method.nameRu == name) {
                return method;
            }
        }
        return std::nullopt;
    }
};

class ITestAddIn {
public:
    virtual ~ITestAddIn() = default;

    virtual void runTests() = 0;

    template<typename T, typename... Args>
    void registerExtension(Args&&... args) {
        static_assert(std::is_base_of_v<IExtentionTestAddIn, T>, "T must be derived from IExtentionTestAddIn");
        registerExtensionImpl(std::make_unique<T>(this, std::forward<Args>(args)...));
    }

    virtual void testCallAsProc(long numProc, std::vector<tVariant>& params) = 0;
    virtual void testCallAsProc(std::u16string nameProc, std::vector<tVariant>& params) = 0;
    virtual void testCallAsFunc(long numFunc, std::vector<tVariant>& params, tVariant& retValue) = 0;
    virtual void testCallAsFunc(std::u16string nameFunc, std::vector<tVariant>& params, tVariant& retValue) = 0;

    virtual IMemoryManager* getMemoryManager() = 0;
	virtual MethodManager* getMethodsInfo() = 0;

protected:

	friend class IExtentionTestAddIn;

    virtual void registerExtensionImpl(std::unique_ptr<IExtentionTestAddIn> extTest) = 0;
};

class TestAddIn : public ITestAddIn {
public:
    TestAddIn(IComponentBase* component) : component_(component) { }

    void runTests() override {
        testInit();
        testSetMemManager();
        testGetInfo();
        testRegisterExtensionAs();
        tesProps();
        testMethods();
        testSetLocale();
        // К сожалению, не все внешние компоненты используют последнюю версию с UserLanguageBase
		// поэтому тестирование этой функции может привести к ошибке Exception thrown: read access violation.
        // testSetUserInterfaceLanguageCode();

		// Run extension test cases
		for (auto& extTest : extTests_) {
			if (!extTest.get()->runTest()) {
				wconsole << L"Extension test failed" << std::endl;
			}
		}

        testDone();
    }

	IMemoryManager* getMemoryManager() override {
		return &memoryManager_;
	}

    MethodManager* getMethodsInfo() override {
		return &methodsNames_;
	}

private:
    void testInit() {
        wconsole << L"Testing Init... ";
        if (component_->Init(&addInDefBase_)) {
            wconsole << L"Testing Init - Success" << std::endl;
        }
        else {
            wconsole << L"Testing Init - Failed" << std::endl;
        }
    }

    void testSetMemManager() {
        wconsole << L"Testing setMemManager... ";
        if (component_->setMemManager(&memoryManager_)) {
            wconsole << L"Testing setMemManager - Success" << std::endl;
        }
        else {
            wconsole << L"Testing setMemManager - Failed" << std::endl;
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
            wconsole << L"Testing RegisterExtensionAs - Success, Extension Name: " << str_utils::to_wstring(extensionName) << std::endl;
			memoryManager_.FreeMemory(reinterpret_cast<void**>(&extensionName));
        }
        else {
            wconsole << L"Testing RegisterExtensionAs - Failed" << std::endl;
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
            wconsole << str_utils::to_wstring(propNameEn) << L" / " << str_utils::to_wstring(propNameRu) << std::endl;
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

			FREE_MEMORY_CONST(memoryManager_, propNameEn);
			FREE_MEMORY_CONST(memoryManager_, propNameRu);
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
                auto hasRetValue = component_->HasRetVal(nMethodIndex);
                long nParams = component_->GetNParams(nMethodIndex);

                methodsNames_.addMethod({ nameMethodEn, nameMethodRu, nMethodIndex, hasRetValue, nParams });

                auto methodIndex = component_->FindMethod(nameMethodRu);
				if (methodIndex == -1) {
					wconsole << L"Fail - Method not found" << std::endl;
				}
				else if (methodIndex != nMethodIndex) {
					wconsole << L"Fail - Method index mismatch: " << methodIndex << std::endl;
				}

                wconsole << L"Method[" << nMethodIndex << L"] : ";
                wconsole << str_utils::to_wstring(nameMethodEn) << L" / " << str_utils::to_wstring(nameMethodRu) << std::endl;
                wconsole << L"  Return value:" << hasRetValue << L"  Count parameters: " << nParams << std::endl;
                for (int nParamIndex = 0; nParamIndex < nParams; nParamIndex++) {
                    tVariant var;
                    if (component_->GetParamDefValue(0, 0, &var)) {
                        wconsole << L"  def[" << nParamIndex << L"]= '" << getVariantValue(var) << L"'" << std::endl;
                    }
					FREE_TVARIANT_MEMORY(memoryManager_, var);
                }

				FREE_MEMORY_CONST(memoryManager_, nameMethodEn);
				FREE_MEMORY_CONST(memoryManager_, nameMethodRu);
            }
		}
        catch (const std::exception& e) {
            wconsole << L"Error test Method: " << e.what() << std::endl;
        }
    }


    void testSetLocale() {
        wconsole << L"Testing SetLocale... ";
        component_->SetLocale(u"en_US");
        wconsole << L"Locale set to en_US" << std::endl;
    }

    void testSetUserInterfaceLanguageCode() {
        wconsole << L"Testing SetUserInterfaceLanguageCode... ";
		try {
		    component_->SetUserInterfaceLanguageCode(u"ru");
			wconsole << L"Success Set interface language code" << std::endl;
		}
		catch (const std::exception& e) {
			wconsole << L"Error Set interface language code: " << e.what() << std::endl;
		}
    }

public:
    
    void testCallAsProc(long numProc, std::vector<tVariant>& params) override {
        auto nameMethodEn = component_->GetMethodName(numProc, 0);
        auto nameMethodRu = component_->GetMethodName(numProc, 1);
        wconsole << L"Testing CallAsProc... " << str_utils::to_wstring(nameMethodEn) << L" / " << str_utils::to_wstring(nameMethodRu);
        if (component_->CallAsProc(numProc, params.data(), static_cast<const long>(params.size()))) {
            wconsole << L"CallAsProc - Success" << std::endl;
        }
        else {
            wconsole << L"CallAsProc - Failed" << std::endl;
        }
    }

    void testCallAsProc(std::u16string nameProc, std::vector<tVariant>& params) override {
        wconsole << L"Testing CallAsProc... " << str_utils::to_wstring(nameProc);
        long numProc = component_->FindMethod(nameProc.c_str());
        if (numProc > -1 ) {
			testCallAsProc(numProc, params);
        }
        else {
            wconsole << L"Testing CallAsProc... " << str_utils::to_wstring(nameProc) << L" Failed" << std::endl;
        }
    }

    void testCallAsFunc(long numFunc, std::vector<tVariant>& params, tVariant& retValue) override {
        auto nameMethodEn = component_->GetMethodName(numFunc, 0);
        auto nameMethodRu = component_->GetMethodName(numFunc, 1);
        wconsole << L"Testing CallAsFunc... " << str_utils::to_wstring(nameMethodEn) << " / " << str_utils::to_wstring(nameMethodRu) << std::endl;
        if (component_->CallAsFunc(numFunc, &retValue, params.data(), static_cast<const long>(params.size()))) {
			wconsole << L"Success result : " << getVariantValue(retValue) << std::endl;
            for (size_t index = 0; index < params.size(); ++index) {
                tVariant& var = params[index];
                wconsole << L"     Param[" << index << L"]: " << getVariantValue(var) << std::endl;
            }

        }
        else {
            wconsole << L"Testing CallAsFunc - Failed" << std::endl;
        }
    }

    void testCallAsFunc(std::u16string nameFunc, std::vector<tVariant>& params, tVariant& retValue) override {
        long numFunc = component_->FindMethod(nameFunc.c_str());
        if (numFunc > -1) {
			testCallAsFunc(numFunc, params, retValue);
        }
		else {
			wconsole << L"Testing CallAsFunc... " << str_utils::to_wstring(nameFunc) << L" Failed" << std::endl;
		}
    }

protected:

    void registerExtensionImpl(std::unique_ptr<IExtentionTestAddIn> extTest) override {
        extTests_.push_back(std::move(extTest));
    }

    friend class IExtentionTestAddIn;

    IComponentBase* component_;
	AddInDefBaseStub addInDefBase_;
	MemoryManagerStub memoryManager_;
    std::vector<std::unique_ptr<IExtentionTestAddIn>> extTests_;
	std::unordered_map<std::u16string, std::u16string> propNames_;
    MethodManager methodsNames_;
};

class ExtensionTestAddInDriverBase : public IExtentionTestAddIn {
public:
    ExtensionTestAddInDriverBase(ITestAddIn* extTest) : IExtentionTestAddIn(extTest) {}
    
	bool runTest() override {
        bool resultTest = true;
        wconsole << L"Start testing AddIn Driver base function... " << std::endl;

        resultTest = resultTest && _testGetInterfaceRevision();
		resultTest = resultTest && _testGetDescription();
		resultTest = resultTest && _testGetParameters();
		resultTest = resultTest && _testGetAdditionalActions();
		wconsole << L"End testing AddIn Driver base function... " << std::endl;

		return resultTest;
	}

private:

    long interfaceRevision_ = 0;
    std::wstring descriptionDriver_;
    std::wstring equipmentType_;
    std::wstring versionDriver_;

	bool _testGetInterfaceRevision() {

		bool result = false;
        interfaceRevision_ = 0;
        versionDriver_ = {};

        auto methodGetVersion = this->extTest_->getMethodsInfo()->findMethodByName(u"GetVersion");
        if (methodGetVersion.has_value()) {
            std::vector<tVariant> params;
            tVariant retValue;
            try {
                extTest_->testCallAsFunc(u"GetVersion", params, retValue);
                if (isValueString(retValue)) {
                    versionDriver_ = getStringValue(retValue);
                    FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), retValue);
                    result = true;
                }
            }
            catch (const std::exception& e) {
                wconsole << L"Error execute GetVersion: " << e.what() << std::endl;
                return false;
            }
        }
        else {
            auto methodGetVersion = this->extTest_->getMethodsInfo()->findMethodByName(u"GetInterfaceRevision");
            if (methodGetVersion.has_value()) {
                INIT_TVARIANT_VECTOR(params, methodGetVersion.value().countParams);
                tVariant retValue;

                try {
                    extTest_->testCallAsFunc(u"GetInterfaceRevision", params, retValue);
                    if (isValueString(retValue)) {
                        interfaceRevision_ = getLongValue(retValue);
                        result = true;
                    }
                }
                catch (const std::exception& e) {
                    wconsole << L"Error execute GetInterfaceRevision: " << e.what() << std::endl;
                    return false;
                }
            }
        }
		return result;
    }

	bool _parseDescription() {

		if (descriptionDriver_.empty()) {
			wconsole << L"  Fail parse, description driver is empty" << std::endl;
			return false;
		}

        std::wregex rgx(L"(EquipmentType=\"([^\"]+)\")");
        std::wsmatch match;

        // Search for the match
        if (std::regex_search(descriptionDriver_, match, rgx)) {
            std::wstring value = match[2].str();
            auto devType = getDeviceTypeDescription(value);
			if (devType.has_value()) {
				equipmentType_ = devType.value();
				return true;
			}
			else {
                wconsole << L"  Error, unknown EquipmentType : " << value << L" on xml description" << std::endl;
			}
        }
        else {
            wconsole << L"  Error, no EquipmentType attribute found in xml description!" << std::endl;
        }
		return false;
	}

    bool _testGetDescription() {

		bool result = false;

        auto methodGetVersion = this->extTest_->getMethodsInfo()->findMethodByName(u"GetDescription");
        if (!methodGetVersion.has_value()) {
            return result;
        }

		INIT_TVARIANT_VECTOR(params, methodGetVersion.value().countParams);
		tVariant retValue;
		try {
			extTest_->testCallAsFunc(u"GetDescription", params, retValue);
			if (TV_VT(&retValue) == VTYPE_BOOL) {
				result = TV_BOOL(&retValue);
			}
			if (result) {
                if (methodGetVersion.value().countParams == 7) {
					auto isInterfaceRevision = getBoolValue(params[4]);
					auto isMainDriver = getBoolValue(params[5]);
					this->interfaceRevision_ = getLongValue(params[3]);
					equipmentType_ = getStringValue(params[2]);
                    std::wstring descriptionDriver = L"Наименование: " + getStringValue(params[0]) + L"\n"
                        + L"Описание: " + getStringValue(params[1]) + L"\n"
                        + L"ТипОборудования: " + equipmentType_ + L"\n"
                        + L"РевизияИнтерфейса: " + std::to_wstring(interfaceRevision_) + L"\n"
                        + L"ИнтеграционнаяБиблиотека: " + ((isInterfaceRevision) ? L"Да" : L"Нет") + L"\n"
                        + L"ОсновнойДрайверУстановлен: " + ((isMainDriver) ? L"Да" : L"Нет") + L"\n"
                        + L"URLCкачивания: " + getStringValue(params[6]) + L"\n";
                    FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), params[0]);
                    FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), params[1]);
                    FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), params[2]);
                    FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), params[6]);
					descriptionDriver_ = descriptionDriver;
                }
                else if (methodGetVersion.value().countParams == 1 ) {
					descriptionDriver_ = getStringValue(params[0]);
					FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), params[0]);
					result = _parseDescription();
				}
			}
		}
		catch (const std::exception& e) {
			wconsole << L"Error execute GetDescription: " << e.what() << std::endl;
			return false;
		}
		return result;
    }

    bool _testGetParameters() {
        bool result = false;
        tVariant retValue;

		if (interfaceRevision_ < 4000) {

            auto methodGetVersion = this->extTest_->getMethodsInfo()->findMethodByName(u"GetParameters");
            if (!methodGetVersion.has_value()) {
                return result;
            }

            INIT_TVARIANT_VECTOR(params, methodGetVersion.value().countParams);
            try {
                extTest_->testCallAsFunc(u"GetParameters", params, retValue);
                if (TV_VT(&retValue) == VTYPE_BOOL) {
                    result = TV_BOOL(&retValue);
                }
            }
            catch (const std::exception& e) {
                wconsole << L"Error execute GetParameters: " << e.what() << std::endl;
                return false;
            }
        }
        else {

            auto methodGetVersion = this->extTest_->getMethodsInfo()->findMethodByName(u"EquipmentParameters");
            if (!methodGetVersion.has_value()) {
                return result;
            }

            INIT_TVARIANT_VECTOR(params, methodGetVersion.value().countParams);


			if (equipmentType_.empty() || methodGetVersion.value().countParams!=2) {
				wconsole << L"  Fail, test GetParameters equipment type is empty" << std::endl;
				return false;
			}

			TV_VT(&params[0]) = VTYPE_PWSTR;
			auto strEqName = str_utils::to_u16string(equipmentType_);
			params[0].pwstrVal = strEqName.data();
            try {
                extTest_->testCallAsFunc(u"EquipmentParameters", params, retValue);
                if (TV_VT(&retValue) == VTYPE_BOOL) {
                    result = TV_BOOL(&retValue);
                }
            }
            catch (const std::exception& e) {
                wconsole << L"Error execute EquipmentParameters: " << e.what() << std::endl;
                return false;
            }
        }
        return result;
    }

    bool _testGetAdditionalActions() {
        bool result = false;
        tVariant retValue;

        wconsole << L"Testing GetAdditionalActions... " << std::endl;

        // Find the method information for "GetAdditionalActions"
        auto methodInfo = this->extTest_->getMethodsInfo()->findMethodByName(u"GetAdditionalActions");
        if (!methodInfo.has_value()) {
            wconsole << L"  Fail, method GetAdditionalActions not found" << std::endl;
            return false;
        }

        // Initialize parameters for the method
        INIT_TVARIANT_VECTOR(params, methodInfo.value().countParams);

        try {
            // Call the method
            extTest_->testCallAsFunc(u"GetAdditionalActions", params, retValue);

            // Check the return value
            if (TV_VT(&retValue) == VTYPE_BOOL) {
                result = TV_BOOL(&retValue);
            }

            if (result) {
                // Extract the XML table of actions
                if (methodInfo.value().countParams == 1 && TV_VT(&params[0]) == VTYPE_PWSTR) {
                    std::wstring actionsXml = getStringValue(params[0]);
                    wconsole << L"  Actions XML: " << actionsXml << std::endl;

                    // Example validation: Check if the XML contains the expected structure
                    if (actionsXml.find(L"<Actions>") != std::wstring::npos) {
                        wconsole << L"  Success, valid XML structure found" << std::endl;
                    }
                    else {
                        wconsole << L"  Fail, invalid XML structure" << std::endl;
                        result = false;
                    }

                    // Free memory for the parameter
                    FREE_TVARIANT_MEMORY(*extTest_->getMemoryManager(), params[0]);
                }
                else {
                    wconsole << L"  Fail, unexpected parameter type or count" << std::endl;
                    result = false;
                }
            }
            else {
                wconsole << L"  Fail, method returned false" << std::endl;
            }
        }
        catch (const std::exception& e) {
            wconsole << L"  Error executing GetAdditionalActions: " << e.what() << std::endl;
            return false;
        }

        return result;
    }

};

#endif // COMPONENT_BASE_TESTER_H