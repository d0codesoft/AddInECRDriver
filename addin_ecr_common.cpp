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
#endif

#include <stdio.h>
#include <wchar.h>
#include <string>
#include "string_conversion.h"
#include "common_types.h"
#include "common_main.h"
#include "string_conversion.h"

#define TIME_LEN 65

#define BASE_ERRNO     7

#ifdef WIN32
#pragma setlocale("ru-RU" )
#endif

static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

// CAddInNative
//---------------------------------------------------------------------------//
CAddInECRCommon::CAddInECRCommon()
{
    m_iMemory = nullptr;
    m_iConnect = nullptr;

	std::wstring logDirectory = getLogDriverFilePath();
    descriptionDriver = {
        L"ECRCommonSC",
        L"Драйвер фискального оборудования по протоколу ECR (electronic cash registrar) протокол ПриватБанк (JSON based)",
        L"POSTerminal",
        true,
        true,
        L"0.1",
        L"1.0.3.1",
        false,
        true,
        true,
        L"https://privatbank.ua/business/sistema-online-navchannya",
        L"",
        true,
        logDirectory
    };

}

//---------------------------------------------------------------------------//
CAddInECRCommon::~CAddInECRCommon()
{
}

//---------------------------------------------------------------------------//
bool CAddInECRCommon::Init(void* pConnection)
{
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInECRCommon::GetInfo()
{
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000;
}

//---------------------------------------------------------------------------//
// this function execute on final delete component in 1C 8.3 or when component stand
void CAddInECRCommon::Done()
{
	m_iConnect = nullptr;
	m_iMemory = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInECRCommon::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
    const wchar_t* wsExtension = L"AddInECRCommonSC";
    size_t iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if (m_iMemory->AllocMemory((void**)wsExtensionName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInECRCommon::GetNProps()
{
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{
    long plPropNum = -1;
    wchar_t* propName = 0;

    ::convFromShortWchar(&propName, wsPropName);
    plPropNum = findName(g_PropNames, propName, ePropLast);

    if (plPropNum == -1)
        plPropNum = findName(g_PropNamesRu, propName, ePropLast);

    delete[] propName;

    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{
    if (lPropNum >= ePropLast)
        return NULL;

    wchar_t* wsCurrentName = NULL;
    WCHAR_T* wsPropName = NULL;
    size_t iActualSize = 0;

    switch (lPropAlias)
    {
    case 0: // First language
        wsCurrentName = (wchar_t*)g_PropNames[lPropNum];
        break;
    case 1: // Second language
        wsCurrentName = (wchar_t*)g_PropNamesRu[lPropNum];
        break;
    default:
        return 0;
    }

    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
    switch (lPropNum)
    {
    case ePropIsEnabled:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = m_boolEnabled;
        break;
    case ePropIsTimerPresent:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = true;
        break;
    case ePropLocale:
    {
        if (m_iMemory)
        {
            TV_VT(pvarPropVal) = VTYPE_PWSTR;
            WCHAR_T* wsPropName = NULL;
            if (m_iMemory->AllocMemory((void**)&(pvarPropVal->pwstrVal), (unsigned)(m_userLang.size() + 1) * sizeof(WCHAR_T)))
            {
                memcpy(pvarPropVal->pwstrVal, m_userLang.data(), m_userLang.size() * sizeof(WCHAR_T));
                pvarPropVal->wstrLen = m_userLang.size();
            }
        }
        else
            TV_VT(pvarPropVal) = VTYPE_EMPTY;
    }
    break;
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
    std::map<UINT, PropName>::iterator it = m_PropNames.find(lPropNum);
    if (it != m_PropNames.end() && !m_PropNames[lPropNum].getParamDefValueFunc) {
        return m_PropNames[lPropNum].getParamDefValueFunc(varPropVal);
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::IsPropReadable(const long lPropNum)
{
    std::map<UINT, PropName>::iterator it = m_PropNames.find(lPropNum);
    if (it != m_PropNames.end()) {
        return m_PropNames[lPropNum].isWritable;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::IsPropWritable(const long lPropNum)
{
    std::map<UINT, PropName>::iterator it = m_PropNames.find(lPropNum);
    if (it != m_PropNames.end()) {
        return m_PropNames[lPropNum].isReadable;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInECRCommon::GetNMethods()
{
    return this->m_MethodNames.size();
}
//---------------------------------------------------------------------------//
long CAddInECRCommon::FindMethod(const WCHAR_T* wsMethodName)
{
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    delete[] name;

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInECRCommon::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
    if (lMethodNum >= eMethLast)
        return NULL;

    wchar_t* wsCurrentName = NULL;
    WCHAR_T* wsMethodName = NULL;
    size_t iActualSize = 0;

    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsMethodName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInECRCommon::GetNParams(const long lMethodNum)
{
    

    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::GetParamDefValue(const long lMethodNum, const long lParamNum,
    tVariant* pvarParamDefValue)
{
    TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::HasRetVal(const long lMethodNum)
{
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::CallAsProc(const long lMethodNum,
    tVariant* paParams, const long lSizeArray)
{
    IAddInDefBaseEx* cnn = (IAddInDefBaseEx*)m_iConnect;
    if (eAppCapabilities2 <= g_capabilities && cnn)
    {
        IAttachedInfo* con_info = (IAttachedInfo*)cnn->GetInterface(eIAttachedInfo);
        if (con_info && con_info->GetAttachedInfo() == IAttachedInfo::eAttachedIsolated)
        {
            //host connected
        }
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInECRCommon::CallAsFunc(const long lMethodNum,
    tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    return true;
}
//---------------------------------------------------------------------------//
void CAddInECRCommon::SetLocale(const WCHAR_T* loc)
{
#if !defined( __linux__ ) && !defined(__APPLE__)
    _wsetlocale(LC_ALL, (wchar_t*)loc);
#else
    //We convert in char* char_locale
    //also we establish locale
    //setlocale(LC_ALL, char_locale);
#endif
}
/////////////////////////////////////////////////////////////////////////////
// UserLanguageBase
//---------------------------------------------------------------------------//
void ADDIN_API CAddInECRCommon::SetUserInterfaceLanguageCode(const WCHAR_T* lang)
{
    m_userLang.assign(lang);
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInECRCommon::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInECRCommon::addError(uint32_t wcode, const wchar_t* source,
    const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T* err = 0;
        WCHAR_T* descr = 0;

        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
void CAddInECRCommon::addError(uint32_t wcode, const char16_t* source, const char16_t* descriptor, long code)
{
    if (m_iConnect)
    {
        m_iConnect->AddError(wcode, source, descriptor, code);
    }
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(const wchar_t* names[], const wchar_t* name,
    const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}

void CAddInECRCommon::initPropNames()
{

}

void CAddInECRCommon::initMethodNames()
{
    this->m_MethodNames = 
    {
        {0, {0, u"GetInterfaceRevision", u"ПолучитьРевизиюИнтерфейса", u"Возвращает поддерживаемую версию требований для данного типа оборудования", false, .callAsFuncFunc = GetInterfaceRevision}},
        {1, {1, u"GetDescription", u"ПолучитьОписание", u"Возвращает информацию о драйвере", {u"DriverDescription"}, {u"STRING[OUT]"}, u"BOOL"}},
        {2, {2, u"GetLastError", u"ПолучитьОшибку", u"Возвращает код и описание последней произошедшей ошибки", {u"ErrorDescription"}, {u"STRING[OUT]"}, u"LONG"}},
        {3, {3, u"EquipmentParameters", u"ПараметрыОборудования", u"Возвращает список параметров настройки драйвера и их типы, значения по умолчанию и возможные значения", {u"EquipmentType"}, {u"STRING[IN]"}, u"BOOL"}},
        {4, {4, u"ConnectEquipment", u"ПодключитьОборудование", u"Подключает оборудование с текущими значениями параметров. Возвращает идентификатор подключенного экземпляра устройства", {u"DeviceID", u"EquipmentType", u"ConnectionParameters"}, {u"STRING[OUT]", u"STRING[IN]", u"STRING[IN]"}, u"BOOL"}},
        {5, {5, u"DisconnectEquipment", u"ОтключитьОборудование", u"Отключает оборудование", {u"DeviceID"}, {u"STRING[IN]"}, u"BOOL"}},
        {6, {6, u"EquipmentTest", u"ТестированиеОборудования", u"Выполняет пробное подключение и опрос устройства с текущими значениями параметров, установленными функцией «УстановитьПараметр». При успешном выполнении подключения в описании возвращается информация об устройстве", {u"EquipmentType", u"ConnectionParameters", u"Description"}, {u"STRING[IN]", u"STRING[IN]", u"STRING[OUT]"}, u"BOOL"}},
        {7, {7, u"EquipmentAutoSetup", u"АвтонастройкаОборудования", u"Выполняет авто-настройку оборудования. Драйвер может показывать технологическое окно, в котором производится автонастройка оборудования. В случае успеха драйвер возвращает параметры подключения оборудования, установленные в результате авто-настройки", {u"EquipmentType", u"ConnectionParameters", u"ConnectionParameters"}, {u"STRING[OUT]", u"STRING[IN]", u"STRING[OUT]"}, u"BOOL"}},
        {8, {8, u"SetApplicationInformation", u"УстановитьИнформациюПриложения", u"Метод передает в драйвер информацию о приложении, в котором используется данный драйвер", {u"ApplicationSettings"}, {u"STRING[IN]"}, u"BOOL"}},
        {9, {9, u"GetAdditionalActions", u"ПолучитьДополнительныеДействия", u"Получает список действий, которые будут отображаться как дополнительные пункты меню в форме настройки оборудования, доступной администратору. Если действий не предусмотрено, возвращает пустую строку", {u"TableActions"}, {u"STRING[OUT]"}, u"BOOL"}},
        {10, {10, u"DoAdditionalAction", u"ВыполнитьДополнительноеДействие", u"Команда на выполнение дополнительного действия с определенным именем", {u"ActionName"}, {u"STRING[IN]"}, u"BOOL"}},
        {11, {11, u"GetLocalizationPattern", u"ПолучитьШаблонЛокализации", u"Возвращает шаблон локализации, содержащий идентификаторы тестовых ресурсов для последующего заполнения", {u"LocalizationPattern"}, {u"STRING[OUT]"}, u"BOOL"}},
        {12, {12, u"SetLocalization", u"УстановитьЛокализацию", u"Устанавливает для драйвера код языка для текущего пользователя и шаблон локализации для текущего пользователя", {u"LanguageCode", u"LocalizationPattern"}, {u"STRING[IN]", u"STRING[IN]"}, u"BOOL"}}
    };
}

/////////////////////////////////////////////////////////////////////////////
// 

LONG CAddInECRCommon::GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    // Ensure no parameters are expected
    if (lSizeArray != 0)
    {
        addError(ADDIN_E_VERY_IMPORTANT, u"GetDescription", u"Method does not accept parameters", -1);
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

bool CAddInECRCommon::GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
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