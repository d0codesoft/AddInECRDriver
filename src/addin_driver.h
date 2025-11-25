#pragma once

#ifndef ADDINECRCOMMONSC_H
#define ADDINECRCOMMONSC_H

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"
#include <map>
#include "common_types.h"
#include "interface_addin_base.h"
#include "interface_driver_base.h"

class CAddInECRDriver :
    public IComponentBase, public IAddInBase
{
public:
    CAddInECRDriver(void);
    virtual ~CAddInECRDriver();
    // IInitDoneBase
    bool ADDIN_API Init(void*) override;
    bool ADDIN_API setMemManager(void* mem) override;
    long ADDIN_API GetInfo() override;
    void ADDIN_API Done() override;
    // ILanguageExtenderBase
    bool ADDIN_API RegisterExtensionAs(WCHAR_T**) override;
    long ADDIN_API GetNProps() override;
    long ADDIN_API FindProp(const WCHAR_T* wsPropName) override;
    const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias) override;
    bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal) override;
    bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal) override;
    bool ADDIN_API IsPropReadable(const long lPropNum) override;
    bool ADDIN_API IsPropWritable(const long lPropNum) override;
    long ADDIN_API GetNMethods() override;
    long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override;
    const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum,
        const long lMethodAlias) override;
    long ADDIN_API GetNParams(const long lMethodNum) override;
    bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
        tVariant* pvarParamDefValue) override;
    bool ADDIN_API HasRetVal(const long lMethodNum) override;
    bool ADDIN_API CallAsProc(const long lMethodNum,
        tVariant* paParams, const long lSizeArray) override;
    bool ADDIN_API CallAsFunc(const long lMethodNum,
        tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    // LocaleBase
    void ADDIN_API SetLocale(const WCHAR_T* loc) override;
    // UserLanguageBase
    void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T* lang) override;

    void sendError(const std::u16string& source, const std::u16string& descriptor,
        UiAddinError codeMsg = UiAddinError::Fail, FacilityCode code = FacilityCode::None) override;

    void sendMsg(const std::u16string& source, const std::u16string& descriptor,
        UiAddinError codeMsg = UiAddinError::Ordinary) override;

    bool getString1C(const std::u16string& source, WCHAR_T** value, uint32_t& length) override;
    bool getString(const WCHAR_T* source, std::u16string& desct) override;
    bool setStringValue(tVariant* pvarParamDefValue, const std::u16string& source) override;
	bool setDoubleValue(tVariant* pvarParamDefValue, const double source) override;
	bool setBoolValue(tVariant* pvarParamDefValue, const bool flag) override;
    bool setNullValue(tVariant* pvarParamDefValue) override;
    bool setIntValue(tVariant* pvarParamDefValue, const int value) override;

	// IAddInBase
    bool saveValue(const std::u16string& key, const std::u16string& value) override;
    bool saveValue(const std::u16string& key, const int value) override;
    bool saveValue(const std::u16string& key, const bool value) override;
    bool loadValue(const std::u16string& key, std::u16string& value) override;
    bool loadValue(const std::u16string& key, int& value) override;
    bool loadValue(const std::u16string& key, bool& value) override;

    bool ExternalEvent(const std::u16string& message, const std::u16string& data) override;
    bool ExternalEvent(const std::wstring& message, const std::wstring& data) override;
    long GetEventBufferDepth() override;
    void SetEventBufferDepth(long depth) override;
    void CleanEventBuffer() override;

    LanguageCode getLanguageCode() override;
	void setDefaultLanguage(LanguageCode lang) override;

    IAddInDefBase* getAddInDefBase() const override;
    IMemoryManager* getMemoryManager() const override;

	HostPlatformInfo getHostPlatformInfo() const;

private:

    bool SetParam(tVariant* pvarParamDefValue, const ParamDefault* defaultParam);
	void initializeDriver();

    void freeValue(tVariant* pvarValue);
    void _setLanguageCode(LanguageCode lang);

    // Attributes
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;

    std::unique_ptr<IDriver1CUniBase> m_driver;

    std::u16string      m_userLang;
	std::u16string      m_locale;
	LanguageCode 	    m_langCode;
    HostPlatformInfo    m_platformInfo;
};

#endif // ADDINECRCOMMONSC_H