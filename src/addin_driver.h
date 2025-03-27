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
    virtual bool ADDIN_API Init(void*) override;
    virtual bool ADDIN_API setMemManager(void* mem) override;
    virtual long ADDIN_API GetInfo() override;
    virtual void ADDIN_API Done() override;
    // ILanguageExtenderBase
    virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**) override;
    virtual long ADDIN_API GetNProps() override;
    virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName) override;
    virtual const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias) override;
    virtual bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal) override;
    virtual bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal) override;
    virtual bool ADDIN_API IsPropReadable(const long lPropNum) override;
    virtual bool ADDIN_API IsPropWritable(const long lPropNum) override;
    virtual long ADDIN_API GetNMethods() override;
    virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override;
    virtual const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum,
        const long lMethodAlias) override;
    virtual long ADDIN_API GetNParams(const long lMethodNum) override;
    virtual bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
        tVariant* pvarParamDefValue) override;
    virtual bool ADDIN_API HasRetVal(const long lMethodNum) override;
    virtual bool ADDIN_API CallAsProc(const long lMethodNum,
        tVariant* paParams, const long lSizeArray) override;
    virtual bool ADDIN_API CallAsFunc(const long lMethodNum,
        tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    // LocaleBase
    virtual void ADDIN_API SetLocale(const WCHAR_T* loc) override;
    // UserLanguageBase
    virtual void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T* lang) override;

    virtual void addError(uint32_t wcode, const std::u16string& source,
        const std::u16string& descriptor, long code) override;

    virtual bool getString1C(const std::u16string& source, WCHAR_T** value, uint32_t& length) override;
    virtual bool getString(const WCHAR_T* source, std::u16string& desct) override;
    virtual bool setStringValue(tVariant* pvarParamDefValue, const std::u16string& source) override;
	virtual bool setBoolValue(tVariant* pvarParamDefValue, const bool flag) override;

	// IAddInBase
    bool saveValue(const std::u16string& key, const std::u16string& value) override;
    bool saveValue(const std::u16string& key, const int value) override;
    bool saveValue(const std::u16string& key, const bool value) override;
    bool loadValue(const std::u16string& key, std::u16string& value) override;
    bool loadValue(const std::u16string& key, int& value) override;
    bool loadValue(const std::u16string& key, bool& value) override;

    LanguageCode getLanguageCode() override;

    IAddInDefBase* getAddInDefBase() const override;
    IMemoryManager* getMemoryManager() const override;

private:

    bool SetParam(tVariant* pvarParamDefValue, const ParamDefault* defaultParam);


    // Attributes
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;

    std::unique_ptr<IDriver1CUniBase> m_driver;

    std::u16string      m_userLang;
	std::u16string      m_locale;
	LanguageCode 	    m_langCode;
};

#endif // ADDINECRCOMMONSC_H