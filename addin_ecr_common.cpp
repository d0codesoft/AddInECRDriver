#include "addin_ecr_common.h"

#if defined( __linux__ ) || defined(__APPLE__)
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
#include "common_main.h"
#include "resource.h"
#include <filesystem>
#include <iostream>

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

	std::string logDirectory = getLogDriverFilePath();
}

//---------------------------------------------------------------------------//
CAddInECRDriver::~CAddInECRDriver()
{
}

std::string get_full_path(const std::string& path)
{
	std::string path_folder = LoadStringResource(IDS_DRIVER_LOG_NAME);
	std::string full = path_folder + path;
	
    try {
        // Check if the directory exists
        std::filesystem::path dir(path_folder);
        if (!std::filesystem::exists(dir)) {
            // Create the directory and all subdirectories
            std::filesystem::create_directories(dir);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem errors
        std::cerr << "Filesystem error: " << e.what() << std::endl;
		full = "";
    }
    catch (const std::exception& e) {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
		full = "";
        // You can also log the error or take other appropriate actions
    }

	return full;
}

// Static initialization of descriptionDriver
DriverDescription CAddInECRDriver::descriptionDriver = {
    LoadStringResource(IDS_DRIVER_NAME),
    LoadStringResource(IDS_DRIVER_DESCRIPTION),
    LoadStringResource(IDS_EQUIPMENT_TYPE),
    false, // IntegrationComponent
    false, // MainDriverInstalled
    LoadStringResource(IDS_DRIVER_VERSION),
    LoadStringResource(IDS_DRIVER_INTEGRATION_COMPONENT_VERSION),
    false, // IsEmulator
    true, // LocalizationSupported
    false, // AutoSetup
    LoadStringResource(IDS_DRIVER_DOWNLOAD_URL),
    LoadStringResource(IDS_DRIVER_ENVIRONMENT_INFORMATION),
    true, // LogIsEnabled
    get_full_path(LoadStringResource(IDS_DRIVER_LOG_NAME))
};

//---------------------------------------------------------------------------//
bool CAddInECRDriver::Init(void* pConnection)
{
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetInfo()
{
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
    const std::u16string wsExtension = u"AddInECRDriverJSON";
    size_t iActualSize = wsExtension.size() + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if (m_iMemory->AllocMemory((void**)wsExtensionName, (unsigned)iActualSize * sizeof(WCHAR_T)))
        {
            // Set all allocated memory to 0
            memset(*wsExtensionName, 0, iActualSize * sizeof(WCHAR_T));
            for (size_t i = 0; i < iActualSize; ++i)
            {
                (*wsExtensionName)[i] = static_cast<WCHAR_T>(wsExtension[i]);
            }
            return true;
        }
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetNProps()
{
    return this->m_PropNames.size();
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::FindProp(const WCHAR_T* wsPropName)
{
    long plPropNum = -1;
	auto prop_name = utf16ToUtf8(wsPropName);

	for (auto& prop : this->m_PropNames)
	{
		if (prop_name == prop.second.name_en || prop_name == prop.second.name_ru)
		{
			plPropNum = prop.second.propId;
			break;
		}
	}
    
    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInECRDriver::GetPropName(long lPropNum, long lPropAlias)
{
    auto it = m_PropNames.find(lPropNum);
    if (it == m_PropNames.end())
        return NULL;

    PropName prop = it->second;

    std::string wsCurrentName = NULL;
    WCHAR_T* wsPropName = NULL;
    size_t iActualSize = 0;

    switch (lPropAlias)
    {
    case 0: // First language
        wsCurrentName = prop.name_en;
        break;
    case 1: // Second language
        wsCurrentName = prop.name_ru;
        break;
    default:
        return 0;
    }

    iActualSize = wsCurrentName.size() + 1;

    if (m_iMemory && !wsCurrentName.empty())
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, (unsigned)iActualSize * sizeof(WCHAR_T)))
        {
            memset(wsPropName, 0, iActualSize * sizeof(WCHAR_T));
            for (size_t i = 0; i < iActualSize; ++i)
            {
                wsPropName[i] = static_cast<WCHAR_T>(wsCurrentName[i]);
            }
        }
    }

    return wsPropName;
}

//---------------------------------------------------------------------------//
bool CAddInECRDriver::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
    auto it = m_PropNames.find(lPropNum);
    if (it == m_PropNames.end())
        return false;

    PropName prop = it->second;
	return prop.getGetPropValFunc(pvarPropVal);
}

//---------------------------------------------------------------------------//
bool CAddInECRDriver::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
    std::map<UINT, PropName>::iterator it = m_PropNames.find(lPropNum);
    if (it != m_PropNames.end() && !it->second.getParamDefValueFunc) {
        return it->second.getParamDefValueFunc(varPropVal);
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::IsPropReadable(const long lPropNum)
{
    std::map<UINT, PropName>::iterator it = m_PropNames.find(lPropNum);
    if (it != m_PropNames.end()) {
        return it->second.isReadable;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::IsPropWritable(const long lPropNum)
{
    std::map<UINT, PropName>::iterator it = m_PropNames.find(lPropNum);
    if (it != m_PropNames.end()) {
        return it->second.isWritable;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetNMethods()
{
    return this->m_MethodNames.size();
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::FindMethod(const WCHAR_T* wsMethodName)
{
	auto methodName = utf16ToUtf8(wsMethodName);

	for (auto& method : this->m_MethodNames)
	{
		if (methodName == method.second.name_en || methodName == method.second.name_ru)
		{
			return method.second.methodId;
		}
	}

    return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInECRDriver::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
	auto it = m_MethodNames.find(lMethodNum);
	if (it == m_MethodNames.end())
		return NULL;

	MethodName method = it->second;

	std::string wsCurrentName = NULL;

    switch (lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = method.name_en;
        break;
    case 1: // Second language
        wsCurrentName = method.name_ru;
        break;
    default:
        return NULL;
    }

    auto iActualSize = wsCurrentName.size() + 1;

    WCHAR_T* wsMethodName = NULL;

    if (m_iMemory && !wsCurrentName.empty())
    {
        if (m_iMemory->AllocMemory((void**)&wsMethodName, (unsigned)iActualSize * sizeof(WCHAR_T)))
        {
            memset(wsMethodName, 0, iActualSize * sizeof(WCHAR_T));
            for (size_t i = 0; i < iActualSize; ++i)
            {
                wsMethodName[i] = static_cast<WCHAR_T>(wsCurrentName[i]);
            }
        }
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInECRDriver::GetNParams(const long lMethodNum)
{
	auto it = m_MethodNames.find(lMethodNum);
    if (it == m_MethodNames.end())
        return 0;

	return (long)it->second.paramCount;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::GetParamDefValue(const long lMethodNum, const long lParamNum,
    tVariant* pvarParamDefValue)
{
    TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::HasRetVal(const long lMethodNum)
{
    auto it = m_MethodNames.find(lMethodNum);
    if (it == m_MethodNames.end())
        return false;

    return it->second.hasRetVal;
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::CallAsProc(const long lMethodNum,
    tVariant* paParams, const long lSizeArray)
{
	auto it = m_MethodNames.find(lMethodNum);

	if (it == m_MethodNames.end())
		return false;

	return it->second.callAsProcFunc(paParams, lSizeArray);
}
//---------------------------------------------------------------------------//
bool CAddInECRDriver::CallAsFunc(const long lMethodNum,
    tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	auto it = m_MethodNames.find(lMethodNum);

	if (it == m_MethodNames.end())
		return false;

	return it->second.callAsFuncFunc(pvarRetValue, paParams, lSizeArray);
}
//---------------------------------------------------------------------------//
void CAddInECRDriver::SetLocale(const WCHAR_T* loc)
{
#if !defined( __linux__ ) && !defined(__APPLE__)
    _wsetlocale(LC_ALL, (wchar_t*)loc);
#else
    char* char_locale = ConvertWCharToChar(loc);
    setenv("LANG", char_locale, 1);    // Set environment variable
    setenv("LC_ALL", char_locale, 1);  // Apply to all locales
    setlocale(LC_ALL, char_locale);
    free(char_locale); // Free the allocated memory
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
}

/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInECRDriver::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}

//---------------------------------------------------------------------------//
void CAddInECRDriver::addError(uint32_t wcode, const std::string source,
    const std::string descriptor, long code)
{
    if (m_iConnect)
    {
		auto err = utf8ToUtf16(source);
		auto descr = utf8ToUtf16(descriptor);

        m_iConnect->AddError(wcode, err.c_str(), descr.c_str(), code);
    }
}

void CAddInECRDriver::initPropNames()
{

}

void CAddInECRDriver::initMethodNames()
{
	this->m_MethodNames = {
		{0, {0, "GetInterfaceRevision", "ПолучитьРевизиюИнтерфейса",
                "Возвращает поддерживаемую версию требований для данного типа оборудования", 
                true, 0, NULL, &CAddInECRDriver::GetInterfaceRevision}}
	};
}

/////////////////////////////////////////////////////////////////////////////
// 

bool CAddInECRDriver::GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    // Ensure no parameters are expected
    if (lSizeArray != 0)
    {
        addError(ADDIN_E_VERY_IMPORTANT, "GetDescription", "Method does not accept parameters", -1);
        return false;
    }

    // 2000 revision is the minimum supported revision 2.0
	// 2004 revision is the maximum supported revision 2.4
	// 4004 revision is the maximum supported revision 4.4

    tVarInit(pvarRetValue);
    TV_VT(pvarRetValue) = VTYPE_I4;
    TV_I4(pvarRetValue) = 4004;

    return true; 
}

bool CAddInECRDriver::GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    // Ensure no parameters are expected
    if (lSizeArray != 0)
    {
        addError(ADDIN_E_VERY_IMPORTANT, u"GetDescription", u"Method does not accept parameters", -1);
        return false;
    }

    // Convert the structure to a JSON string
    std::wstring jsonDescription = convertDriverDescriptionToJson(this->descriptionDriver);
    size_t iActualSize = jsonDescription.size() + 1;
    WCHAR_T* wsDescription = nullptr;

    if (m_iMemory)
    {
        if (m_iMemory->AllocMemory((void**)&wsDescription, (unsigned)iActualSize * sizeof(WCHAR_T)))
        {
            ::convToShortWchar(&wsDescription, jsonDescription.c_str(), iActualSize);
            TV_VT(pvarRetValue) = VTYPE_PWSTR;
            pvarRetValue->pwstrVal = wsDescription;
            pvarRetValue->wstrLen = iActualSize - 1;
            return true;
        }
    }

    return false;
}