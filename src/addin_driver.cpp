#include "pch.h"
#include "addin_driver.h"

#if defined(OS_MACOS) || defined(OS_LINUX)
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <iconv.h>
#include <locale.h>
#include <sys/time.h>
#else
#include <clocale>
#endif

#include <stdio.h>
#include <wchar.h>
#include <string>
#include "string_conversion.h"
#include "common_types.h"
#include <filesystem>
#include <iostream>
#include "interface_pos_terminal.h"
#include "sys_utils.h"
#include "driver_pos_terminal.h"
#include "logger.h"
#include "str_utils.h"

#define BASE_ERRNO     7

#ifdef WIN32
#pragma setlocale("ru-RU")
#endif

static AppCapabilities g_capabilities = eAppCapabilities3;

// CAddInNative
//---------------------------------------------------------------------------//
CAddInECRDriver::CAddInECRDriver()
{
    m_iMemory = nullptr;
    m_iConnect = nullptr;
	m_langCode = LanguageCode::RU;

	//std::u16string logDirectory = SysUtils::getLogDriverFilePath();
	m_driver = std::make_unique<DriverPOSTerminal>(this);
}

//---------------------------------------------------------------------------//
CAddInECRDriver::~CAddInECRDriver()
{
	if (m_driver)
        m_driver.reset();
}

//---------------------------------------------------------------------------//
bool CAddInECRDriver::Init(void* pConnection)
{
    SysUtils::LogInfo(L"AddIn Init");

    if (!pConnection) return false;
    m_iConnect = static_cast<IAddInDefBase*>(pConnection);
	
    if (m_iConnect)
        m_driver->InitDriver();

    return m_iConnect != nullptr;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetInfo()
{
    SysUtils::LogInfo(L"AddIn GetInfo");
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000;
}

//---------------------------------------------------------------------------//
// this function execute on final delete component in 1C 8.3 or when component stand
void CAddInECRDriver::Done()
{
	m_iConnect = nullptr;
	m_iMemory = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInECRDriver::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
    SysUtils::LogInfo(L"AddIn RegisterExtensionAs");
    std::u16string extensionName = wstringToU16string(this->m_driver->getDescriptionDriver().ExtensionName);

	uint32_t iSize = 0;
    if (getString1C(extensionName, wsExtensionName, iSize)) {
		return true;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetNProps()
{
    return static_cast<long>(this->m_driver->GetProperties().size());
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::FindProp(const WCHAR_T* wsPropName)
{
    long plPropNum = -1;
    std::u16string prop_name;
	if (getString(wsPropName, prop_name)) {
		return plPropNum;
	}

    const auto& properties = this->m_driver->GetProperties();

    auto it = std::find_if(properties.begin(), properties.end(),
        [&](const PropName& prop) {
            return prop_name == prop.name_en || prop_name == prop.name_ru;
        });

    if (it != properties.end()) {
        size_t index = std::distance(properties.begin(), it);
        plPropNum = static_cast<long>(index);
    }
    
    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInECRDriver::GetPropName(long lPropNum, long lPropAlias)
{
    const auto& properties = this->m_driver->GetProperties();
	if (properties.empty() && lPropNum >= properties.size())
		return NULL;

    std::u16string wsCurrentName = {};

    switch (lPropAlias)
    {
    case 0: // First language
        wsCurrentName = properties[lPropNum].name_en;
        break;
    case 1: // Second language
        wsCurrentName = properties[lPropNum].name_ru;
        break;
    default:
        return NULL;
    }

    uint32_t iActualSize = 0;
    WCHAR_T* wsPropName = nullptr;
    if (getString1C(wsCurrentName, &wsPropName, iActualSize)) {
        return wsPropName;
    }

    return NULL;
}

//---------------------------------------------------------------------------//
bool CAddInECRDriver::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
    const auto& properties = this->m_driver->GetProperties();
	if (properties.empty() && lPropNum >= properties.size())
        return false;
	
    if (!properties[lPropNum].getPropValFunc)
		return false;

	return properties[lPropNum].getPropValFunc(pvarPropVal);
}

//---------------------------------------------------------------------------//
bool CAddInECRDriver::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
    const auto& properties = this->m_driver->GetProperties();
    if (properties.empty() && lPropNum >= properties.size())
        return false;

    if (!properties[lPropNum].setPropValFunc)
        return false;
    
    return properties[lPropNum].setPropValFunc(varPropVal);
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::IsPropReadable(const long lPropNum)
{
    const auto& properties = this->m_driver->GetProperties();
    if (properties.empty() && lPropNum >= properties.size())
        return false;

    return properties[lPropNum].isReadable;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::IsPropWritable(const long lPropNum)
{
    const auto& properties = this->m_driver->GetProperties();
    if (properties.empty() && lPropNum >= properties.size())
        return false;

    return properties[lPropNum].isWritable;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetNMethods()
{
    const auto& methodNames = m_driver->GetMethods();
    return static_cast<long>(methodNames.size());
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::FindMethod(const WCHAR_T* wsMethodName)
{
	long plMethodNum = -1;
    const auto& methods = m_driver->GetMethods();
	if (methods.empty())
		return plMethodNum;

    std::u16string methodName = {};
	if (!getString(wsMethodName, methodName)) {
		return plMethodNum;
	}

    auto it = std::find_if(methods.begin(), methods.end(),
        [&](const MethodName& method) {
            return methodName == method.name_en || methodName == method.name_ru;
        });


    if (it != methods.end()) {
        size_t index = std::distance(methods.begin(), it);
        plMethodNum = static_cast<long>(index);
    }

	return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInECRDriver::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
    const auto& methods = m_driver->GetMethods();

	if (methods.empty() && lMethodNum >= methods.size())
		return NULL;
	
    std::u16string wsCurrentName = {};

    switch (lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = methods[lMethodNum].name_en;
        break;
    case 1: // Second language
        wsCurrentName = methods[lMethodNum].name_ru;
        break;
    default:
        return NULL;
    }

    uint32_t iActualSize = 0;
    WCHAR_T* wsMethodName = nullptr;
    if (getString1C(wsCurrentName, &wsMethodName, iActualSize)) {
		return wsMethodName;
    }

    return NULL;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetNParams(const long lMethodNum)
{
    const auto& methods = m_driver->GetMethods();
	if (methods.empty() && lMethodNum >= methods.size())
		return 0;
	
	return static_cast<long>(methods[lMethodNum].paramCount);
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::GetParamDefValue(const long lMethodNum, const long lParamNum,
    tVariant* pvarParamDefValue)
{
    TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

    const auto& methods = m_driver->GetMethods();
    if (methods.empty() && lMethodNum >= methods.size())
        return false;

	auto itParam = methods[lMethodNum].default_value_param.find(lParamNum);
    if (itParam == methods[lMethodNum].default_value_param.end())
        return true;

    return SetParam(pvarParamDefValue, &itParam->second);
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::HasRetVal(const long lMethodNum)
{
    const auto& methods = m_driver->GetMethods();
    if (methods.empty() && lMethodNum >= methods.size())
        return false;

    return methods[lMethodNum].hasRetVal;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::CallAsProc(const long lMethodNum,
    tVariant* paParams, const long lSizeArray)
{
    const auto& methods = m_driver->GetMethods();
    if (methods.empty() && lMethodNum >= methods.size())
        return false;


	if (std::holds_alternative<CallAsProc1C>(methods[lMethodNum].ptr_method)) {
		return std::get<CallAsProc1C>(methods[lMethodNum].ptr_method)(paParams, lSizeArray);
	}
    
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::CallAsFunc(const long lMethodNum,
    tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    const auto& methods = m_driver->GetMethods();
    if (methods.empty() && lMethodNum >= methods.size())
        return false;

    if (std::holds_alternative<CallAsFunc1C>(methods[lMethodNum].ptr_method)) {
        return std::get<CallAsFunc1C>(methods[lMethodNum].ptr_method)(pvarRetValue, paParams, lSizeArray);
    }

    return false;
}
//---------------------------------------------------------------------------//
void CAddInECRDriver::SetLocale(const WCHAR_T* loc)
{
#if !defined( __linux__ ) && !defined(__APPLE__)
    _wsetlocale(LC_ALL, (wchar_t*)loc);
    std::u16string locale = std::u16string(loc);
	m_langCode = detectLanguage(locale);
#else
    char* char_locale = ConvertWCharToChar(loc);
    setenv("LANG", char_locale, 1);    // Set environment variable
    setenv("LC_ALL", char_locale, 1);  // Apply to all locales
    setlocale(LC_ALL, char_locale);
    free(char_locale); // Free the allocated memory
    
    std::u16string locale = str_utils::to_u16string(loc);
    m_langCode = detectLanguage(locale);
    //We convert in char* char_locale
    //also we establish locale
    //setlocale(LC_ALL, char_locale);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// UserLanguageBase
//---------------------------------------------------------------------------//
void ADDIN_API CAddInECRDriver::SetUserInterfaceLanguageCode(const WCHAR_T* lang)
{
    m_userLang.assign(lang);
    std::u16string locale = std::u16string(lang);
    m_langCode = detectLanguage(locale);
}

/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInECRDriver::setMemManager(void* mem)
{
	if (!mem)
		return false;

    m_iMemory = static_cast<IMemoryManager*>(mem);
    return m_iMemory != nullptr;
}

//---------------------------------------------------------------------------//
void CAddInECRDriver::addError(uint32_t wcode, const std::u16string& source,
    const std::u16string& descriptor, long code)
{
    if (m_iConnect)
    {
        m_iConnect->AddError(wcode, source.c_str(), descriptor.c_str(), code);
    }
}

IAddInDefBase* CAddInECRDriver::getAddInDefBase() const
{
    return m_iConnect;
}

IMemoryManager* CAddInECRDriver::getMemoryManager() const
{
    return m_iMemory;
}

bool CAddInECRDriver::getString1C(const std::u16string& source, WCHAR_T** value, uint32_t& length)
{
    size_t iActualSize = source.size() + 1;

    if (m_iMemory && m_iMemory->AllocMemory((void**)value, static_cast<uint32_t>(iActualSize) * sizeof(WCHAR_T)))
    {
        memset(*value, 0, iActualSize * sizeof(WCHAR_T));
		for (size_t i = 0; i < source.size(); i++)
		{
			(*value)[i] = (WCHAR_T)source[i];
		}
        //memcpy(*value, source.c_str(), source.size() * sizeof(char16_t));
        //(*value)[source.size()] = 0;
        length = static_cast<uint32_t>(source.size());
        return true;
    }
    return false;
}

bool CAddInECRDriver::getString(const WCHAR_T* source, std::u16string& dest)
{
    if (!source) {
        return false;
    }

    size_t length = 0;
    while (source[length] != 0) {
        ++length;
    }

    dest.assign(source, length);
    return true;
}

bool CAddInECRDriver::setStringValue(tVariant* pvarParamDefValue, const std::u16string& source) {
    if (!pvarParamDefValue) {
        return false;
    }
    TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
    if (!getString1C(source, &pvarParamDefValue->pwstrVal, pvarParamDefValue->wstrLen)) {
        TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
        return false;
    }
    return true;
}

bool CAddInECRDriver::setBoolValue(tVariant* pvarParamDefValue, const bool flag) {
	if (!pvarParamDefValue) {
		return false;
	}
	TV_VT(pvarParamDefValue) = VTYPE_BOOL;
	TV_BOOL(pvarParamDefValue) = flag;
	return true;
}

bool CAddInECRDriver::saveValue(const std::u16string& key, const std::u16string& value)
{
	if (m_iConnect) {
        tVariant var;
        TV_VT(&var) = VTYPE_PWSTR;
        TV_WSTR(&var) = const_cast<char16_t*>(value.c_str());
		return m_iConnect->Write(const_cast<char16_t*>(key.c_str()), &var);
    }

    return false;
}

bool CAddInECRDriver::saveValue(const std::u16string& key, const int value)
{
    if (m_iConnect) {
        tVariant var;
        TV_VT(&var) = VTYPE_I4;
        TV_I4(&var) = value;
        return m_iConnect->Write(const_cast<char16_t*>(key.c_str()), &var);
    }
	return false;
}

bool CAddInECRDriver::saveValue(const std::u16string& key, const bool value)
{
    if (m_iConnect) {
        tVariant var;
        TV_VT(&var) = VTYPE_BOOL;
        TV_BOOL(&var) = value;
        return m_iConnect->Write(const_cast<char16_t*>(key.c_str()), &var);
    }
	return false;
}

bool CAddInECRDriver::loadValue(const std::u16string& key, std::u16string& value)
{
	if (m_iConnect) {
		tVariant var;
        long errorCode = 0;
        WCHAR_T* errorDesc = nullptr;

		if (m_iConnect->Read(const_cast<char16_t*>(key.c_str()), &var, &errorCode, &errorDesc)) {
			if (var.vt == VTYPE_PWSTR) {
				value = std::u16string(var.pwstrVal);
				return true;
			}
		}
        else {
            if (errorCode != 0) {
                std::u16string _errorDesc = std::u16string(errorDesc);
                std::wstring werrorDesc = u16stringToWstring(_errorDesc);
                std::wstring wnameProp = u16stringToWstring(key);
                std::wstring wErrorCode = std::to_wstring(errorCode);
                LOG_ERROR_ADD(L"AddIn", L"Error read property '" + wnameProp + L"' : " + werrorDesc + L" Code " + wErrorCode);
            }
        }
	}

	return false;
}

bool CAddInECRDriver::loadValue(const std::u16string& key, int& value)
{
	if (m_iConnect) {
		tVariant var;
		long errorCode = 0;
		WCHAR_T* errorDesc = nullptr;
		if (m_iConnect->Read(const_cast<char16_t*>(key.c_str()), &var, &errorCode, &errorDesc)) {
			if (var.vt == VTYPE_I4) {
				value = var.intVal;
				return true;
			}
		}
		else {
			if (errorCode != 0) {
				std::u16string _errorDesc = std::u16string(errorDesc);
				std::wstring werrorDesc = u16stringToWstring(_errorDesc);
				std::wstring wnameProp = u16stringToWstring(key);
				std::wstring wErrorCode = std::to_wstring(errorCode);
				LOG_ERROR_ADD(L"AddIn", L"Error read property '" + wnameProp + L"' : " + werrorDesc + L" Code " + wErrorCode);
			}
		}
	}

    return false;
}

bool CAddInECRDriver::loadValue(const std::u16string& key, bool& value)
{
    if (m_iConnect) {
        tVariant var;
        long errorCode = 0;
        WCHAR_T* errorDesc = nullptr;
        if (m_iConnect->Read(const_cast<char16_t*>(key.c_str()), &var, &errorCode, &errorDesc)) {
            if (var.vt == VTYPE_BOOL) {
                value = var.bVal;
                return true;
            }
        }
        else {
            if (errorCode != 0) {
                std::u16string _errorDesc = std::u16string(errorDesc);
                std::wstring werrorDesc = u16stringToWstring(_errorDesc);
                std::wstring wnameProp = u16stringToWstring(key);
                std::wstring wErrorCode = std::to_wstring(errorCode);
                LOG_ERROR_ADD(L"AddIn", L"Error read property '" + wnameProp + L"' : " + werrorDesc + L" Code " + wErrorCode);
            }
        }
    }

    return false;
}

LanguageCode CAddInECRDriver::getLanguageCode()
{
    return m_langCode;
}

bool CAddInECRDriver::SetParam(tVariant* pvarParamDefValue, const ParamDefault* defaultParam)
{
	if (!defaultParam) {
		return true;
	}

    if (std::holds_alternative<int>(*defaultParam)) {
        TV_VT(pvarParamDefValue) = VTYPE_I4;
		long lVal = std::get<int>(*defaultParam);
		TV_I4(pvarParamDefValue) = lVal;
    }
    else if (std::holds_alternative<std::u16string>(*defaultParam)) {
        std::u16string str = std::get<std::u16string>(*defaultParam);
        if (getString1C(str, &pvarParamDefValue->pwstrVal, pvarParamDefValue->wstrLen)) {
            TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
		}
		else {
			TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
			this->addError(ADDIN_E_VERY_IMPORTANT, u"SetParam", u"Error setting default value", -1);
			return false;
		}
	}
//    else if (std::holds_alternative<bool>(*defaultParam)) {
//        TV_VT(pvarParamDefValue) = VTYPE_BOOL;
//        TV_BOOL(pvarParamDefValue) = std::get<bool>(*defaultParam);
//    }
    else {
        this->addError(ADDIN_E_VERY_IMPORTANT, u"SetParam", u"Error setting default value", -1);
        TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
