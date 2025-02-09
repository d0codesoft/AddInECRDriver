#pragma once

#ifndef ADDINECRCOMMONSC_H
#define ADDINECRCOMMONSC_H

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"
#include <map>
#include "common_types.h"

class CAddInECRCommon :
    public IComponentBase
{
public:
    enum Props
    {
        eLastProp      // Always last
    };

    enum Methods
    {
        eLastMethod      // Always last
    };

    CAddInECRCommon(void);
    virtual ~CAddInECRCommon();
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

private:
    // Attributes
    void addError(uint32_t wcode, const wchar_t* source,
        const wchar_t* descriptor, long code);
    void addError(uint32_t wcode, const char16_t* source,
        const char16_t* descriptor, long code);

	void initPropNames();
	void initMethodNames();

	// Methods implement the functionality of the add-in Driver
    LONG GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
    bool GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);

    // Attributes
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;

	std::map<UINT, PropName> m_PropNames;
	std::map<UINT, MethodName> m_MethodNames;
    DriverDescription descriptionDriver;

    bool                m_boolEnabled;
    uint32_t            m_uiTimer;
    std::u16string      m_userLang;
	std::u16string      m_locale;
};

#endif // ADDINECRCOMMONSC_H