#include "pch.h"
#include <iomanip>
#include <string>
#include <functional>
#include <sstream>
#include "driver_pos_terminal.h"
#include "interface_pos_terminal.h"
#include "string_conversion.h"
#include "sys_utils.h"
#include "str_utils.h"
#include "setting_driver_pos.h"
#include "logger.h"
#include "tvariant_helper.h"

#define CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, lValidCountParams, methodName) \
    if ((lSizeArray) != (lValidCountParams) || !(paParams)) { \
        auto slValidCountParams = wstringToU16string(std::to_wstring(lValidCountParams)); \
        std::u16string msg = u"Method expects " + slValidCountParams + u" parameter(s)"; \
        addErrorDriver(msg, u16stringToWstring(methodName) + L": Method expects " + std::to_wstring(lValidCountParams) + L" parameter(s)"); \
        m_addInBase->setBoolValue(pvarRetValue, false); \
        return false; \
    } \


DriverPOSTerminal::DriverPOSTerminal(IAddInBase * addInBase) : m_addInBase(addInBase)
{
	m_licenseManager = std::make_unique<LicenseManager>();
}

DriverPOSTerminal::~DriverPOSTerminal()
{
	if (m_licenseManager)
		m_licenseManager.reset();

    if (m_connection) {
		if (m_connection->isConnected()) {
			m_connection->disconnect();
		}
        m_connection.reset();
    }
}

void DriverPOSTerminal::InitDriver()
{
    auto drivePath = SysUtils::get_full_path(LoadStringResource(L"IDS_DRIVER_LOG_NAME"));
	// Implementation here
    m_driverDescription = {
        LoadStringResource(L"IDS_DRIVER_NAME"),
        LoadStringResource(L"IDS_DRIVER_DESCRIPTION"),
        LoadStringResource(L"IDS_EQUIPMENT_TYPE"),
        false, // IntegrationComponent
        false, // MainDriverInstalled
        LoadStringResource(L"IDS_DRIVER_VERSION"),
        LoadStringResource(L"IDS_DRIVER_INTEGRATION_COMPONENT_VERSION"),
        false, // IsEmulator
        true, // LocalizationSupported
        false, // AutoSetup
        LoadStringResource(L"IDS_DRIVER_DOWNLOAD_URL"),
        LoadStringResource(L"IDS_DRIVER_ENVIRONMENT_INFORMATION"),
        true, // LogIsEnabled
        drivePath,
        LoadStringResource(L"IDS_DRIVER_NAME_ADDIN")
    };

    m_ParamConnection = {
        { L"ConnectionType", L"", TypeParameter::String },
	    { L"Address", L"", TypeParameter::String },
	    { L"Port", 2000, TypeParameter::Number },
		{ L"Speed", 9600, TypeParameter::Number }
    };
}

const DriverDescription& DriverPOSTerminal::getDescriptionDriver()
{
    clearError();
    return m_driverDescription;
}

// Implementing methods from IDriver1CUniBase
bool DriverPOSTerminal::GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Ensure no parameters are expected
    clearError();

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 0, u"GetInterfaceRevision");

    if (lSizeArray != 0)
    {
		this->m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"GetInterfaceRevision", u"Method does not accept parameters", -1);
		addErrorDriver(u"Method does not accept parameters", L"GetInterfaceRevision: Method does not accept parameters");
        return false;
    }

    // 2000 revision is the minimum supported revision 2.0
    // 2004 revision is the maximum supported revision 2.4
	// 3004 revision is the maximum supported revision 2.4
    // 4004 revision is the maximum supported revision 4.4

    tVarInit(pvarRetValue);
    TV_VT(pvarRetValue) = VTYPE_I4;
    TV_I4(pvarRetValue) = DRIVER_REQUIREMENTS_VERSION;

    return true;
}

// Метод: ПолучитьОписание (GetDescription)
// Описание: 
// Возвращает информацию о драйвере в формате XML.
//
// Параметры:
// 0. STRING ОписаниеДрайвера (DriverDescription) [OUT]  
//    XML таблица, содержащая структуру описания драйвера.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если информация о драйвере успешно получена, иначе `false`.
bool DriverPOSTerminal::GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetDescription");

    // Convert the structure to a XML string
    std::u16string xmlDescription = toXml(this->m_driverDescription);
	auto result = m_addInBase->setStringValue(paParams, xmlDescription);
    m_addInBase->setBoolValue(pvarRetValue, result);

	return result;
}

// Метод: ПолучитьОшибку (GetLastError)
// Описание: 
// Возвращает код и описание последней произошедшей ошибки.
//
// Параметры:
// 0. STRING ОписаниеОшибки (ErrorDescription) [OUT]  
//    Текстовое описание последней ошибки.
//
// Возвращаемое значение:
// LONG  
// Возвращает код последней ошибки.
bool DriverPOSTerminal::GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetDescription");

    m_addInBase->setBoolValue(pvarRetValue, true);
    m_addInBase->setStringValue(paParams, m_lastError);
    return true;
}

// Метод: ПолучитьПараметры (GetParameters)
// Описание: 
// Возвращает список параметров настройки драйвера, включая их типы, значения по умолчанию и возможные значения.
//
// Параметры:
// 0. STRING ТаблицаПараметров (TableParameters) [OUT]  
//    XML таблица, содержащая список параметров настройки драйвера.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если список параметров успешно получен, иначе `false`.
bool DriverPOSTerminal::GetParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetParameters");

	auto xmlParam = toXML(SettingDriverPos::getSettings());
	m_addInBase->setStringValue(paParams, xmlParam);
	m_addInBase->setBoolValue(pvarRetValue, true);

    return true;
}

// Метод: УстановитьПараметр (SetParameter)
// Описание: 
// Выполняет установку значения параметра по его имени.
//
// Параметры:
// 0. STRING Имя (Name) [IN]  
//    Имя параметра, значение которого требуется установить.
//
// 1. VARIANT Значение (Value) [IN]  
//    Значение параметра. Тип данных определяется в зависимости от параметра.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если значение параметра успешно установлено, иначе `false`.
bool DriverPOSTerminal::SetParameter(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"SetParameter");

	auto paramName = m_addInBase->getStringValue(paParams[0]);

    if (VariantHelper::isValueString(paParams[1])) {
        auto valStr = VariantHelper::getStringValue(paParams[1]);
		if (valStr.has_value()) {
            setParameterValue(m_ParamConnection, paramName, valStr.value());
        }
	}
	else if (VariantHelper::isValueInt(paParams[1])) {
        auto valInt = VariantHelper::getIntValue(paParams[1]);
        if (valInt.has_value()) {
            setParameterValue(m_ParamConnection, paramName, valInt.value());
        }
	}
	else if (paParams[1].vt == VTYPE_BOOL) {
        auto valBool = VariantHelper::getBoolValue(paParams[1]);
        if (valBool.has_value()) {
            setParameterValue(m_ParamConnection, paramName, valBool.value());
        }
	}
	else {
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

    m_addInBase->setBoolValue(pvarRetValue, true);
	return true;
}

// Метод: Подключить (Open)
// Описание: 
// Подключает оборудование с текущими значениями параметров, установленными функцией «УстановитьПараметр».  
// Возвращает идентификатор подключенного экземпляра устройства.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [OUT]  
//    Идентификатор подключенного экземпляра устройства.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если подключение выполнено успешно, иначе `false`.
bool DriverPOSTerminal::Open(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"Open");

    std::wstring error = {}, deviceId = {};
    auto result = InitConnection(deviceId, error);
    if (result)
    {
        m_addInBase->setStringValue(paParams, str_utils::to_u16string(deviceId));
    }
    else {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"Open", str_utils::to_u16string(error), -1);
        addErrorDriver(u"Invalid connect", L"Open: " + error);
        TV_VT(paParams) = VTYPE_EMPTY;
    }

    m_addInBase->setBoolValue(pvarRetValue, result);

    return false;
}

// Метод: Отключить (Close)
// Описание: 
// Отключает оборудование, используя переданный идентификатор устройства.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства, которое необходимо отключить.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если оборудование успешно отключено, иначе `false`.
bool DriverPOSTerminal::Close(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"Close");

    std::wstring deviceID;
    if (paParams->vt == VTYPE_PWSTR) {
        deviceID = str_utils::to_wstring(paParams->pwstrVal);
    }
    else {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"Close", u"Invalid type for device ID", -1);
        addErrorDriver(u"Invalid type device ID", L"Close: Invalid type for device ID");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    auto findId = m_connections.find(deviceID);
    if (findId == m_connections.end()) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"Close", u"Invalid device ID", -1);
        addErrorDriver(u"Invalid device ID", L"Close: Invalid device ID");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    if (findId->second != nullptr)
    {
        if (findId->second.get()->isConnected())
        {
            findId->second.get()->disconnect();
        }
        findId->second.reset();
    }
    m_connections.erase(findId);

    m_addInBase->setBoolValue(pvarRetValue, true);
    return true;
}

// Метод: ТестУстройства (DeviceTest)
// Описание: 
// Выполняет пробное подключение и опрос устройства с текущими значениями параметров, установленными функцией «УстановитьПараметр».  
// При успешном выполнении подключения в параметре "Описание" возвращается информация об устройстве.  
// В случае активного демонстрационного режима возвращает описание его ограничений.
//
// Параметры:
// 0. STRING Описание (Description) [OUT]  
//    Описание результата выполнения теста. Если тест успешен, содержит информацию об устройстве.
//
// 1. STRING АктивированДемоРежим (DemoModeIsActivated) [OUT]  
//    Описание ограничений демонстрационного режима при его наличии.  
//    Если режим отсутствует, возвращает пустую строку.  
//    Пример: «Драйвер является платным, и для полноценной работы нужен ключ защиты.»
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если устройство успешно прошло тестирование, иначе `false`.
bool DriverPOSTerminal::DeviceTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"DeviceTest");

	// Test connection
	auto result = testConnection(m_ParamConnection);

	if (m_licenseManager->isDemoMode()) {
        m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(m_licenseManager->getDemoModeDescription()));
	}
	else {
		TV_VT(&paParams[1]) = VTYPE_EMPTY;
	}

    m_addInBase->setBoolValue(pvarRetValue, result);
    return result;
}


//************************************************************
// Метод: ПараметрыОборудования (EquipmentParameters)
// Описание: 
// Возвращает список параметров настройки драйвера оборудования, включая их типы, значения по умолчанию и возможные значения.
//
// Параметры:
// 0. STRING ТипОборудования (EquipmentType) [IN]  
//    Тип оборудования. Имеет одно из значений из таблицы "Типоборудования".
//
// 1. STRING ТаблицаПараметров (TableParameters) [OUT]  
//    XML таблица, содержащая список параметров настройки оборудования.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция выполнена успешно, иначе `false`.
//************************************************************
bool DriverPOSTerminal::EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"EquipmentParameters");

	auto type = getEquipmentTypeInfoFromVariant(&paParams[0]);
	if (!type.has_value() || type.value() != EquipmentTypeInfo::POSTerminal) {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid type for EquipmentType", -1);
		addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

    // Get SettingXML
	auto settingXml = SettingDriverPos::getSettingXML();
	m_addInBase->setStringValue(&paParams[1], settingXml);
    m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
}

//************************************************************
// Метод: ПодключитьОборудование (ConnectEquipment)
// Описание: 
// Подключает оборудование с текущими значениями параметров и возвращает идентификатор подключенного экземпляра устройства.
//
// Параметры:
// 0. STRING ТипОборудования (EquipmentType) [IN]  
//    Тип оборудования. Имеет одно из значений из таблицы "Типоборудования".
//
// 1. STRING ПараметрыПодключения (ConnectionParameters) [IN]  
//    XML таблица, содержащая параметры подключения оборудования.
//
// 2. STRING ИДУстройства (DeviceID) [OUT]  
//    Идентификатор подключенного экземпляра устройства.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если подключение выполнено успешно, иначе `false`.
//************************************************************
bool DriverPOSTerminal::ConnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 3, u"ConnectEquipment");

    // Get EquipmentType (STRING[IN])
    auto type = getEquipmentTypeInfoFromVariant(&paParams[0]);
    if (!type.has_value() || type.value() != EquipmentTypeInfo::POSTerminal) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid type for EquipmentType", -1);
        addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    // Get ConnectionParameters (STRING[IN])
	auto paramConnect = VariantHelper::getStringValue(paParams[1]);
    if (!paramConnect.has_value()) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Invalid type for ConnectionParameters", -1);
        addErrorDriver(u"Invalid type for ConnectionParameters", L"ConnectEquipment: Invalid type for ConnectionParameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    // Parce xml parametrs
	if (!ParseParametersFromXML(m_ParamConnection, paramConnect.value()))
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Invalid ConnectionParameters", -1);
        addErrorDriver(u"Invalid ConnectionParameters", L"ConnectEquipment: Invalid ConnectionParameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
	}

	std::wstring error, deviceId;

    auto result = InitConnection(deviceId, error);
	if (result)
	{
        m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(deviceId));
	}
    else {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", str_utils::to_u16string(error), -1);
        addErrorDriver(u"Invalid ConnectionParameters", L"ConnectEquipment: " + error);
        TV_VT(&paParams[2]) = VTYPE_EMPTY;
    }

	m_addInBase->setBoolValue(pvarRetValue, result);
    return result;
}

//************************************************************
// Метод: ОтключитьОборудование (DisconnectEquipment)
// Описание: 
// Отключает оборудование, используя переданный идентификатор устройства.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства, которое необходимо отключить.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если оборудование успешно отключено, иначе `false`.
//************************************************************
bool DriverPOSTerminal::DisconnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"DisconnectEquipment");

    std::wstring deviceID;
    if (paParams->vt == VTYPE_PWSTR) {
        deviceID = str_utils::to_wstring(paParams->pwstrVal);
	}
	else {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"DisconnectEquipment", u"Invalid type for device ID", -1);
		addErrorDriver(u"Invalid type for device ID", L"DisconnectEquipment: Invalid type for device ID");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
	}

	auto findId = m_connections.find(deviceID);
	if (findId == m_connections.end()) {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"DisconnectEquipment", u"Invalid device ID", -1);
		addErrorDriver(u"Invalid device ID", L"DisconnectEquipment: Invalid device ID");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

	if (findId->second != nullptr)
	{
		if (findId->second.get()->isConnected())
		{
            findId->second.get()->disconnect();
		}
        findId->second.reset();
	}
	m_connections.erase(findId);
    
    m_addInBase->setBoolValue(pvarRetValue, true);
    return true;
}

//********************************************************************************************************************
// Метод: ТестированиеОборудования (EquipmentTest)
// Описание: 
// Выполняет пробное подключение и опрос устройства с текущими значениями параметров, установленными функцией «УстановитьПараметр».  
// При успешном выполнении подключения в параметре "Описание" возвращается информация об устройстве.  
// В случае активного демонстрационного режима возвращает описание его ограничений.
//
// Параметры:
// 0. STRING ТипОборудования (EquipmentType) [IN]  
//    Тип оборудования. Имеет одно из значений из таблицы "Типоборудования".
//
// 1. STRING ПараметрыПодключения (ConnectionParameters) [IN]  
//    XML-таблица, содержащая параметры подключения устройства.
//
// 2. STRING Описание (Description) [OUT]  
//    Описание результата выполнения теста. Если тест успешен, содержит информацию об устройстве.
//
// 3. STRING АктивированДемоРежим (DemoModeIsActivated) [OUT]  
//    Описание ограничений демонстрационного режима при его наличии.  
//    Если режим отсутствует, возвращает пустую строку.  
//    Пример: «Драйвер является платным, и для полноценной работы нужен ключ защиты.»
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если устройство успешно прошло тестирование, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {

    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"EquipmentTest");

   // Get EquipmentType (STRING[IN])
    auto type = getEquipmentTypeInfoFromVariant(&paParams[0]);
    if (!type.has_value() || type.value() != EquipmentTypeInfo::POSTerminal) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentTest", u"Invalid type for EquipmentType", -1);
        addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentTest: Invalid type for EquipmentType");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

	// Get ConnectionParameters (STRING[IN])
	auto connectionParameters = VariantHelper::getStringValue(paParams[1]);
    if (!connectionParameters.has_value()) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentTest", u"Invalid type for ConnectionParameters", -1);
        addErrorDriver(u"Invalid type for ConnectionParameters", L"EquipmentTest: Invalid type for ConnectionParameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

	std::vector<DriverParameter> paramConnection;
    if (!ParseParametersFromXML(paramConnection, connectionParameters.value()))
    {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentTest", u"Invalid ConnectionParameters", -1);
        addErrorDriver(u"Invalid ConnectionParameters", L"EquipmentTest: Invalid ConnectionParameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    if (m_licenseManager->isDemoMode()) {
        m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(m_licenseManager->getDemoModeDescription()));
    }
    else {
        TV_VT(&paParams[1]) = VTYPE_EMPTY;
    }

	// Test connection
	auto result = testConnection(paramConnection);
    m_addInBase->setBoolValue(pvarRetValue, result);

	if (!result) {
        addErrorDriver(u"Invalid test connection", L"EquipmentTest: Error connection");
    }

    return result;
}

//********************************************************************************************************************
// Метод: АвтонастройкаОборудования (EquipmentAutoSetup)
// Описание: 
// Выполняет автоматическую настройку оборудования.  
// Драйвер может отображать технологическое окно, в котором производится автонастройка оборудования.  
// В случае успеха драйвер возвращает параметры подключения, установленные в результате автонастройки.
//
// Параметры:
// 0. STRING ПараметрыПодключения (ConnectionParameters) [IN]  
//    XML-таблица, содержащая параметры подключения оборудования по умолчанию.
//
// 1. BOOL ПоказыватьОкноАвтонастройки (ShowAutoSetupWindow) [IN]  
//    Определяет, должно ли отображаться окно автонастройки оборудования.
//
// 2. LONG Таймаут (Timeout) [IN]  
//    Таймаут выполнения операции в миллисекундах.  
//    Если значение `0`, таймаут не ограничен.
//
// 3. STRING ТипОборудования (EquipmentType) [OUT]  
//    Тип оборудования. Имеет одно из значений из таблицы "Типоборудования".
//
// 4. STRING ПараметрыПодключения (ConnectionParameters) [OUT]  
//    XML-таблица, содержащая параметры подключения, установленные в результате автонастройки.
//
// Возвращаемое значение:
// BOOL
// Возвращает `true`, если автонастройка оборудования выполнена успешно, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 5, u"EquipmentAutoSetup");

    

    return false;
}

//********************************************************************************************************************
// Метод: УстановитьИнформациюПриложения (SetApplicationInformation)
// Описание: 
// Передает в драйвер информацию о приложении, в котором используется данный драйвер.
//
// Параметры:
// 0. STRING ПараметрыПриложения (ApplicationSettings) [IN]  
//    XML-строка с параметрами приложения.  
//    Пример структуры XML:  
//    ```xml
//    <ApplicationSettings>
//        <AppName>POSSystem</AppName>
//        <AppVersion>2.5.1</AppVersion>
//        <AppID>12345</AppID>
//    </ApplicationSettings>
//    ```
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если информация о приложении успешно передана в драйвер, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"SetApplicationInformation");

    // Convert the structure to a XML string
    std::u16string xmlDescription = toXmlApplication(this->m_driverDescription);
    auto result = m_addInBase->setStringValue(paParams, xmlDescription);
    m_addInBase->setBoolValue(pvarRetValue, result);

    return result;
}

//********************************************************************************************************************
// Метод: ПолучитьДополнительныеДействия (GetAdditionalActions)
// Описание: 
// Получает список дополнительных действий, которые будут отображаться как пункты меню  
// в форме настройки оборудования, доступной администратору.  
// Если дополнительных действий не предусмотрено, возвращает пустую строку.
//
// Параметры:
// 0. STRING ТаблицаДействий (TableActions) [OUT]  
//    XML-таблица, содержащая список доступных дополнительных действий.  
//    Пример структуры XML:  
//    ```xml
//    <TableActions>
//        <Action>
//            <ActionID>1</ActionID>
//            <ActionName>Обновить прошивку</ActionName>
//            <Description>Позволяет обновить прошивку оборудования до последней версии.</Description>
//        </Action>
//        <Action>
//            <ActionID>2</ActionID>
//            <ActionName>Сброс настроек</ActionName>
//            <Description>Выполняет сброс настроек оборудования к заводским параметрам.</Description>
//        </Action>
//    </TableActions>
//    ```
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если список действий успешно сформирован, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetAdditionalActions");

    // Convert the structure to a XML string
    std::u16string xmlDescription = toXMLActions(this->getActions(), m_addInBase->getLanguageCode());
    auto result = m_addInBase->setStringValue(paParams, xmlDescription);
    m_addInBase->setBoolValue(pvarRetValue, result);

    return false;
}

//********************************************************************************************************************
// Метод: ВыполнитьДополнительноеДействие (DoAdditionalAction)
// Описание: 
// Выполняет дополнительное действие, указанное по имени.  
// Действие должно быть предварительно определено в списке, полученном методом `ПолучитьДополнительныеДействия`.
//
// Параметры:
// 0. STRING ИмяДействия (ActionName) [IN]  
//    Имя действия, которое требуется выполнить.  
//    Должно соответствовать одному из действий из таблицы `GetAdditionalActions`.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если действие успешно выполнено, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"DoAdditionalAction");

    auto actions = this->getActions();
	auto actionName = str_utils::to_u16string(m_addInBase->getStringValue(paParams[0]));
	auto find_action = std::find_if(actions.begin(), actions.end(), [&](const ActionDriver& action) {
		return actionName == action.name_en || actionName == action.name_ru;
		});

	if (find_action == actions.end()) {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"DoAdditionalAction", u"Invalid type for ActionName", -1);
		addErrorDriver(u"Invalid type for ActionName", L"DoAdditionalAction: Invalid type for ActionName");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

	auto result = find_action->action(pvarRetValue, paParams, lSizeArray);

    return result;
}

//********************************************************************************************************************
// Метод: ПолучитьШаблонЛокализации (GetLocalizationPattern)
// Описание: 
// Возвращает шаблон локализации в формате XML, содержащий идентификаторы тестовых ресурсов для последующего заполнения.
//
// Параметры:
// 0. STRING ШаблонЛокализации (LocalizationPattern) [OUT]  
//    XML-строка, содержащая структуру локализации.  
//    Пример структуры XML:  
//    ```xml
//    <Localization>
//        <Resource id="IDS_DRIVER_NAME"/>
//        <Resource id="IDS_DRIVER_DESCRIPTION"/>
//        <Resource id="IDS_DRIVER_VERSION"/>
//    </Localization>
//    ```
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если шаблон локализации успешно сформирован, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetLocalizationPattern");

	// To fu..k with this 
    // Not deatil documentatition for this method
    TV_VT(paParams) = VTYPE_EMPTY;
    this->m_addInBase->setBoolValue(pvarRetValue, false);

    return false;
}

//********************************************************************************************************************
// Метод: УстановитьЛокализацию (SetLocalization)
// Описание: 
// Устанавливает код языка интерфейса и шаблон локализации для текущего пользователя драйвера.
//
// Параметры:
// 0. STRING КодЯзыка (LanguageCode) [IN]  
//    Код языка в формате ISO 639-1 (например, "en" — английский, "ru" — русский).
//
// 1. STRING ШаблонЛокализации (LocalizationPattern) [IN]  
//    XML-строка, содержащая локализационные ресурсы для выбранного языка.  
//    Пример структуры XML:  
//    ```xml
//    <Localization>
//        <Resource id="IDS_DRIVER_NAME">Драйвер POS-терминала</Resource>
//        <Resource id="IDS_DRIVER_DESCRIPTION">Описание драйвера</Resource>
//    </Localization>
//    ```
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если язык и шаблон локализации успешно установлены, иначе `false`.
//********************************************************************************************************************
bool DriverPOSTerminal::SetLocalization(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"SetLocalization");

	// to fu..k with this
	// is not implemented yet
	// Not deatil documentatition for this method
    TV_VT(&paParams[0]) = VTYPE_EMPTY;
    TV_VT(&paParams[1]) = VTYPE_EMPTY;
    this->m_addInBase->setBoolValue(pvarRetValue, false);

    return false;
}

const std::vector<MethodName>& DriverPOSTerminal::GetMethods() {
    clearError();
    return m_MethodNames;
}

const std::vector<PropName>& DriverPOSTerminal::GetProperties() {
    clearError();
    return m_PropNames;
}


// Implementing methods from IDriverPosTerminal

//********************************************************************************************************************
// Метод: ПараметрыТерминала (TerminalParameters)
// Описание: 
// Возвращает параметры работы терминала для указанного устройства в формате XML.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ПараметрыТерминала (TerminalParameters) [OUT]  
//    XML-таблица, содержащая параметры работы терминала.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если параметры терминала успешно получены, иначе `false`.
bool DriverPOSTerminal::TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"TerminalParameters");

	auto deviceId = m_addInBase->getStringValue(paParams[0]);
    auto param = getTerminalConfig(deviceId);
	if (!param.has_value()) {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"TerminalParameters", u"Invalid type for DeviceID", -1);
		addErrorDriver(u"Invalid type for DeviceID", L"TerminalParameters: Invalid type for DeviceID");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

    auto xmlParameter = toXMLTerminalConfig(param.value());
	m_addInBase->setStringValue(&paParams[1], xmlParameter);
	m_addInBase->setBoolValue(pvarRetValue, true);

    return true;
}

//********************************************************************************************************************
// Метод: Оплатить (Pay)
// Описание: 
// Выполняет операцию оплаты на указанном устройстве с заданными параметрами. 
// В случае успешной операции возвращает исходящие параметры и текст квитанции. 
// При отказе операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции оплаты.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после выполнения.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция оплаты выполнена успешно, иначе `false`.
bool DriverPOSTerminal::Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"Pay");



    return false;
}

// Метод: ОплатитьПлатежнойКартой (PayByPaymentCard)
// Описание: 
// Выполняет авторизацию оплаты по платежной карте.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING НомерКарты (CardNumber) [IN/OUT]  
//    Входной параметр — данные карты или номер карты.  
//    Выходной параметр — может измениться в зависимости от эквайринговой системы.
//
// 2. DOUBLE Сумма (Amount) [IN]  
//    Сумма операции по карте.
//
// 3. STRING НомерЧека (ReceiptNumber) [IN/OUT]  
//    Входной параметр — номер чека, переданный в систему.  
//    Выходной параметр — может быть изменен после обработки транзакции.
//
// 4. STRING КодRRN (RRNCode) [OUT]  
//    Уникальный код транзакции RRN.
//
// 5. STRING КодАвторизации (AuthorizationCode) [OUT]  
//    Код авторизации транзакции.
//
// 6. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция успешно выполнена, иначе `false`.
bool DriverPOSTerminal::PayByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"PayByPaymentCard");


    return false;
}

// Метод: ВернутьПлатежПоПлатежнойКарте (ReturnPaymentByPaymentCard)
// Описание: 
// Выполняет возврат платежа, ранее совершенного с использованием платежной карты.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING НомерКарты (CardNumber) [IN/OUT]  
//    Входной параметр — данные карты или номер карты.  
//    Выходной параметр — может измениться в зависимости от эквайринговой системы.
//
// 2. DOUBLE Сумма (Amount) [IN]  
//    Сумма возврата по карте.
//
// 3. STRING НомерЧека (ReceiptNumber) [IN/OUT]  
//    Входной параметр — номер чека оригинальной транзакции.  
//    Выходной параметр — может измениться после обработки возврата.
//
// 4. STRING КодRRN (RRNCode) [IN/OUT]  
//    Уникальный код транзакции RRN.  
//    Если передается, используется для идентификации оригинальной транзакции.
//
// 5. STRING КодАвторизации (AuthorizationCode) [IN/OUT]  
//    Код авторизации оригинальной транзакции.  
//    Может измениться после обработки возврата.
//
// 6. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция возврата успешно выполнена, иначе `false`.
bool DriverPOSTerminal::ReturnPaymentByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"ReturnPaymentByPaymentCard");

    return false;
}

//********************************************************************************************************************
// Метод: ВернутьПлатеж (ReturnPayment)
// Описание: 
// Выполняет возврат платежа с указанными параметрами операции. 
// Возвращает исходящие параметры операции и текст квитанции, сформированный эквайринговым ПО. 
// В случае отказа возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходяшиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после выполнения возврата.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции по операции, сформированный эквайринговым ПО. 
//    В случае отказа возвращается текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если возврат платежа выполнен успешно, иначе `false`.
bool DriverPOSTerminal::ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"ReturnPayment");

    return false;
}

// Метод: ОтменитьПлатеж (CancelPayment)
// Описание: 
// Выполняет отмену платежа на указанном устройстве. 
// Поддерживает частичную отмену (на сумму меньше исходной операции), если терминал поддерживает данную возможность. 
// Возвращает исходящие параметры операции и текст квитанции. В случае отказа возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции отмены.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после отмены.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция отмены выполнена успешно, иначе `false`.
bool DriverPOSTerminal::CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"CancelPayment");
    return false;
}

// Метод: ОтменитьПлатежПоПлатежнойКарте (CancelPaymentByPaymentCard)
// Описание: 
// Выполняет отмену платежа, ранее совершенного с использованием платежной карты.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING НомерКарты (CardNumber) [IN/OUT]  
//    Входной параметр — данные карты или номер карты.  
//    Выходной параметр — может измениться в зависимости от эквайринговой системы.
//
// 2. DOUBLE Сумма (Amount) [IN]  
//    Сумма отменяемого платежа.
//
// 3. STRING НомерЧека (ReceiptNumber) [IN/OUT]  
//    Входной параметр — номер чека оригинальной транзакции.  
//    Выходной параметр — может измениться после обработки отмены.
//
// 4. STRING КодRRN (RRNCode) [IN/OUT]  
//    Уникальный код транзакции RRN.  
//    Если передается, используется для идентификации оригинальной транзакции.
//
// 5. STRING КодАвторизации (AuthorizationCode) [IN/OUT]  
//    Код авторизации оригинальной транзакции.  
//    Может измениться после обработки отмены.
//
// 6. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция отмены успешно выполнена, иначе `false`.
bool DriverPOSTerminal::CancelPaymentByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"CancelPaymentByPaymentCard");

    return false;
}

// Метод: Преавторизация (Authorisation)
// Описание: 
// Выполняет преавторизацию — блокирование указанной суммы на счете пользователя. 
// Возвращает исходящие параметры операции и текст квитанции. 
// В случае отказа операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции преавторизации.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после выполнения преавторизации.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция преавторизации выполнена успешно, иначе `false`.
bool DriverPOSTerminal::Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"Authorisation");

    return false;
}

// Метод: ПреавторизацияПоПлатежнойКарте (AuthorisationByPaymentCard)
// Описание: 
// Выполняет преавторизацию – блокирование указанной суммы на счете карты.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING НомерКарты (CardNumber) [IN/OUT]  
//    Входной параметр — данные карты или номер карты.  
//    Выходной параметр — может измениться в зависимости от эквайринговой системы.
//
// 2. DOUBLE Сумма (Amount) [IN]  
//    Сумма, которая будет заблокирована на карте.
//
// 3. STRING НомерЧека (ReceiptNumber) [IN/OUT]  
//    Входной параметр — номер чека.  
//    Выходной параметр — может измениться после обработки операции.
//
// 4. STRING КодRRN (RRNCode) [OUT]  
//    Уникальный код транзакции RRN.
//
// 5. STRING КодАвторизации (AuthorizationCode) [OUT]  
//    Код авторизации транзакции.
//
// 6. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция преавторизации успешно выполнена, иначе `false`.
bool DriverPOSTerminal::AuthorisationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"AuthorisationByPaymentCard");

    return false;
}

// Метод: ЗавершитьПреавторизацию (AuthConfirmation)
// Описание: 
// Завершает ранее выполненную операцию преавторизации, списывая заблокированную сумму со счета пользователя. 
// Возвращает исходящие параметры операции и текст квитанции. 
// В случае отказа операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции завершения преавторизации.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после списания суммы.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция завершения преавторизации выполнена успешно, иначе `false`.
bool DriverPOSTerminal::AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"AuthConfirmation");

    return false;
}

// Завершает пре-авторизацию — списывает сумму со счета карты
// AuthConfirmationByPaymentCard
// Параметры:
// [0] STRING  DeviceID          [IN]     - Идентификатор устройства
// [1] STRING& CardNumber        [IN/OUT] - Номер карты / Данные карты
// [2] DOUBLE  Amount            [IN]     - Сумма операции
// [3] STRING& ReceiptNumber     [IN/OUT] - Номер чека
// [4] STRING  RRNCode           [IN]     - Уникальный код транзакции RRN
// [5] STRING  AuthorizationCode [IN]     - Код авторизации транзакции
// [6] STRING& Slip              [OUT]    - Текст квитанции, сформированный эквайринговым ПО
bool DriverPOSTerminal::AuthConfirmationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"AuthConfirmationByPaymentCard");

    return false;
}

// Метод: ОтменитьПреавторизацию (CancelAuthorisation)
// Описание: 
// Отменяет ранее выполненную операцию преавторизации, разблокируя сумму на счете пользователя. 
// Возвращает исходящие параметры операции и текст квитанции. 
// В случае отказа операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции отмены преавторизации.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после разблокировки суммы.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция отмены преавторизации выполнена успешно, иначе `false`.
bool DriverPOSTerminal::CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"CancelAuthorisation");

    return false;
}

// Метод: ОтменитьПреавторизациюПоПлатежнойКарте (CancelAuthorisationByPaymentCard)
// Описание: 
// Выполняет отмену преавторизации – разблокировку суммы на счете карты.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING НомерКарты (CardNumber) [IN/OUT]  
//    Входной параметр — данные карты или номер карты.  
//    Выходной параметр — может измениться в зависимости от эквайринговой системы.
//
// 2. DOUBLE Сумма (Amount) [IN]  
//    Сумма, которая должна быть разблокирована.
//
// 3. STRING НомерЧека (ReceiptNumber) [IN/OUT]  
//    Входной параметр — номер чека.  
//    Выходной параметр — может измениться после обработки операции.
//
// 4. STRING КодRRN (RRNCode) [IN]  
//    Уникальный код транзакции RRN, связанный с операцией преавторизации.
//
// 5. STRING КодАвторизации (AuthorizationCode) [IN]  
//    Код авторизации, полученный при выполнении преавторизации.
//
// 6. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если отмена преавторизации успешно выполнена, иначе `false`.
bool DriverPOSTerminal::CancelAuthorisationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"CancelAuthorisationByPaymentCard");

    return false;
}

// Метод: ОплатаCВыдачейНаличных (PayWithCashWithdrawal)
// Описание: 
// Выполняет операцию оплаты с одновременной выдачей наличных денежных средств.  
// Операция выдачи наличных не может выполняться отдельно и должна сопровождаться оплатой по карте.  
// В случае отказа операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции оплаты с выдачей наличных.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после выполнения.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции по операции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция оплаты с выдачей наличных выполнена успешно, иначе `false`.
bool DriverPOSTerminal::PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"PayWithCashWithdrawal");

    return false;
}

// >= DRIVER_REQUIREMENTS_VERSION_4_1
// Метод: ОплатитьПлатежнойКартойCВыдачейНаличных (PayByPaymentCardWithCashWithdrawal)
// Описание: 
// Выполняет операцию оплаты по платежной карте с выдачей наличных денежных средств.  
// Операция выдачи наличных не является самостоятельной и сопровождается обязательной оплатой по карте.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. LONG НомерМерчанта (MerchantNumber) [IN]  
//    Номер мерчанта, доступного для данного эквайрингового терминала.
//
// 2. STRING РеквизитыКартыQR (ConsumerPresentedQR) [IN]  
//    Отсканированный QR-код с мобильного устройства держателя карты, содержащий EMV-реквизиты карты или токена.
//
// 3. DOUBLE СуммаОперации (AmountPayment) [IN]  
//    Сумма оплаты по платежной карте. Значение должно быть больше 0.
//
// 4. DOUBLE СуммаНаличных (AmountCash) [IN]  
//    Сумма выдаваемых наличных в рамках операции.
//
// 5. STRING НомерКарты (CardNumber) [OUT]  
//    Маскированный номер карты или данные карты.
//
// 6. STRING НомерЧека (ReceiptNumber) [OUT]  
//    Номер чека.
//
// 7. STRING КодRRN (RRNCode) [OUT]  
//    Уникальный код транзакции RRN.
//
// 8. STRING КодАвторизации (AuthorizationCode) [OUT]  
//    Код авторизации транзакции.
//
// 9. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции по операции, сформированный эквайринговым ПО.  
//    В случае отказа по операции возвращается текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция успешно выполнена, иначе `false`.
bool DriverPOSTerminal::PayByPaymentCardWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 10, u"PayByPaymentCardWithCashWithdrawal");

    return false;
}

// Метод: ПокупкаСЗачислением (PurchaseWithEnrollment)
// Описание: 
// Выполняет операцию покупки с зачислением денежных средств на карту клиента.  
// В случае отказа операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходящиеПараметры (OperationParameters) [IN]  
//    XML-таблица, содержащая входящие параметры операции покупки с зачислением.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML-таблица, содержащая исходящие параметры операции после выполнения.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции по операции, сформированный эквайринговым ПО.  
//    В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция покупки с зачислением выполнена успешно, иначе `false`.
bool DriverPOSTerminal::PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 3, u"PurchaseWithEnrollment");

    return false;
}

// Метод: ПолучитьПараметрыКарты (GetCardParametrs)
// Описание: 
// Выполняет получение параметров карты. В зависимости от параметра `ОтПоследнейОперации` 
// либо считывает карту через эквайринговый терминал, либо возвращает параметры последней операции.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING РеквизитыКартыQR (ConsumerPresentedQR) [IN]  
//    Отсканированный QR-код с мобильного устройства держателя карты, содержащий EMV-реквизиты карты или токена.
//
// 2. BOOLEAN ОтПоследнейОперации (FromLastOperation) [IN]  
//    Определяет метод получения параметров карты:  
//    - `false`: терминал запрашивает чтение карты.  
//    - `true`: возвращаются параметры последней произведенной операции.
//
// 3. STRING НомерКарты (CardNumber) [OUT]  
//    Номер карты или данные карты.
//
// 4. STRING ХешНомерКарты (CardNumberHash) [OUT]  
//    PAN карты в хешированном виде по алгоритму SHA256.
//
// 5. STRING СсылкаНаПлатежныйСчет (PaymentAccountReference) [OUT]  
//    PAR (Payment Account Reference) — ссылка на платежный счет.
//
// 6. STRING ТипКарты (CardType) [OUT]  
//    Название платежной системы карты (например, Visa).
//
// 7. LONG СвояКарта (IsOwnCard) [OUT]  
//    Указывает принадлежность карты банку эквайрера:  
//    - `0`: Неопределенно  
//    - `1`: Карта принадлежит банку эквайрера  
//    - `2`: Карта НЕ принадлежит банку эквайрера
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если параметры карты успешно получены, иначе `false`.
bool DriverPOSTerminal::GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 8, u"GetCardParameters");

    return false;
}

// Метод: ПолучитьПараметрыКарты (GetCardParametrs)
// Описание: 
// Получает параметры карты. В зависимости от параметра `ОтПоследнейОперации`  
// либо запрашивает данные карты через эквайринговый терминал, либо возвращает параметры последней операции.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING РеквизитыКартыQR (ConsumerPresentedQR) [IN]  
//    Отсканированный QR-код с мобильного устройства держателя карты, содержащий EMV-реквизиты карты или токена.
//
// 2. BOOLEAN ОтПоследнейОперации (FromLastOperation) [IN]  
//    Определяет метод получения параметров карты:  
//    - `false`: терминал запрашивает чтение карты.  
//    - `true`: возвращаются параметры последней произведенной операции.
//
// 3. STRING НомерКарты (CardNumber) [OUT]  
//    Номер карты или данные карты.
//
// 4. STRING ХешНомерКарты (CardNumberHash) [OUT]  
//    PAN карты в хешированном виде по алгоритму SHA256.
//
// 5. STRING СсылкаНаПлатежныйСчет (PaymentAccountReference) [OUT]  
//    При использовании мобильного устройства передается значение Payment Account Reference (при его наличии).
//
// 6. STRING ТипКарты (CardType) [OUT]  
//    Название платежной системы карты (например, Visa).
//
// 7. LONG СвояКарта (IsOwnCard) [OUT]  
//    Указывает принадлежность карты банку эквайрера:  
//    - `0`: Неопределенно  
//    - `1`: Карта принадлежит банку эквайрера  
//    - `2`: Карта НЕ принадлежит банку эквайрера
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если параметры карты успешно получены, иначе `false`.
bool DriverPOSTerminal::GetCardParametrs(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 8, u"GetCardParametrs");

    return false;
}

// Метод: ОплатитьСертификатом (PayCertificate)
// Описание: 
// Выполняет оплату с применением электронного сертификата ФЭС НСПК. 
// В случае отказа по операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходяшиеПараметры (OperationParameters) [IN]  
//    XML таблица, содержащая входящие параметры операции.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML таблица, содержащая исходящие параметры операции.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции по операции, сформированный эквайринговым ПО.  
//    В случае отказа возвращается текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция прошла успешно, иначе `false`.
bool DriverPOSTerminal::PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"PayCertificate");


    return false;
}

// Метод: ОплатитьЭлектроннымСертификатом (PayElectronicCertificate)
// Описание: 
// Выполняет оплату с применением электронного сертификата ФЭС НСПК.  
// В случае отказа по операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. LONG НомерМерчанта (MerchantNumber) [IN]  
//    Номер мерчанта, доступного для данного эквайрингового терминала.
//
// 2. STRING РеквизитыКартыQR (ConsumerPresentedQR) [IN]  
//    Отсканированный QR-код с мобильного устройства держателя карты, содержащий EMV-реквизиты карты или токена.
//
// 3. STRING ИдентификаторКорзины (BasketID) [IN/OUT]  
//    Уникальный идентификатор операции в ФЭС НСПК.
//
// 4. DOUBLE СуммаЭлектроногоСертификата (ElectronicCertificateAmount) [IN]  
//    Сумма операции, покрываемая электронными сертификатами.
//
// 5. DOUBLE СуммаСобственныхСредств (OwnFundsAmount) [IN]  
//    Сумма операции, оплачиваемая личными средствами по карте.
//
// 6. STRING НомерКарты (CardNumber) [OUT]  
//    Маскированный номер карты или данные карты.
//
// 7. STRING НомерЧека (ReceiptNumber) [OUT]  
//    Номер чека.
//
// 8. STRING КодRRN (RRNCode) [OUT]  
//    Уникальный код транзакции RRN.
//
// 9. STRING КодАвторизации (AuthorizationCode) [OUT]  
//    Код авторизации транзакции.
//
// 10. LONG СтатусОперации (OperationStatus) [OUT]  
//     Статус выполнения операции:  
//     - `0`: Операция выполнена полностью  
//     - `1`: Оплата личными средствами по карте не выполнена
//
// 11. STRING ТекстКвитанции (Slip) [OUT]  
//     Текст квитанции по операции, сформированный эквайринговым ПО.  
//     В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция успешно выполнена, иначе `false`.
bool DriverPOSTerminal::PayElectronicCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 12, u"PayElectronicCertificate");

    return false;
}

// Метод: ВернутьСертификатом (ReturnCertificate)
// Описание: 
// Выполняет возврат оплаты с применением электронного сертификата ФЭС НСПК.  
// В случае отказа по операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ВходяшиеПараметры (OperationParameters) [IN]  
//    XML таблица, содержащая входящие параметры операции.
//
// 2. STRING ИсходящиеПараметры (OperationParameters) [OUT]  
//    XML таблица, содержащая исходящие параметры операции.
//
// 3. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции по операции, сформированный эквайринговым ПО.  
//    В случае отказа возвращается текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция прошла успешно, иначе `false`.
bool DriverPOSTerminal::ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 4, u"ReturnCertificate");


    return false;
}

// Метод: ВернутьЭлектроннымСертификатом (ReturnElectronicCertificate)
// Описание: 
// Выполняет возврат оплаты с применением электронного сертификата ФЭС НСПК.  
// В случае отказа по операции возвращается текст квитанции отказа.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. LONG НомерМерчанта (MerchantNumber) [IN]  
//    Номер мерчанта, доступного для данного эквайрингового терминала.
//
// 2. STRING РеквизитыКартыQR (ConsumerPresentedQR) [IN]  
//    Отсканированный QR-код с мобильного устройства держателя карты, содержащий EMV-реквизиты карты или токена.
//
// 3. STRING ИдентификаторКорзины (BasketID) [IN/OUT]  
//    Уникальный идентификатор операции в ФЭС НСПК.  
//    Может быть передан BasketID оригинальной операции оплаты, в таком случае возврат выполняется без требования банковской карты.
//
// 4. DOUBLE СуммаЭлектроногоСертификата (ElectronicCertificateAmount) [IN]  
//    Сумма возврата, покрываемая электронными сертификатами.
//
// 5. DOUBLE СуммаСобственныхСредств (OwnFundsAmount) [IN]  
//    Сумма возврата, покрываемая личными средствами по карте.
//
// 6. STRING НомерКарты (CardNumber) [IN/OUT]  
//    Входной параметр — данные карты или номер карты.  
//    Выходной параметр — маскированный номер карты.
//
// 7. STRING НомерЧека (ReceiptNumber) [IN/OUT]  
//    Входной параметр — номер чека оригинальной транзакции.  
//    Выходной параметр — может измениться после обработки возврата.
//
// 8. STRING КодRRN (RRNCode) [IN/OUT]  
//    Уникальный код транзакции RRN.  
//    Может быть передан для идентификации оригинальной операции.
//
// 9. STRING КодАвторизации (AuthorizationCode) [OUT]  
//    Код авторизации транзакции возврата.
//
// 10. LONG СтатусОперации (OperationStatus) [OUT]  
//     Статус выполнения операции:  
//     - `0`: Операция выполнена полностью  
//     - `1`: Возврат личных средств по карте не выполнен
//
// 11. STRING ТекстКвитанции (Slip) [OUT]  
//     Текст квитанции по операции, сформированный эквайринговым ПО.  
//     В случае отказа операции содержит текст квитанции отказа.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция успешно выполнена, иначе `false`.
bool DriverPOSTerminal::ReturnElectronicCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 12, u"ReturnElectronicCertificate");

    return false;
}

// Метод: АварийнаяОтменаОперации (EmergencyReversal)
// Описание: 
// Выполняет отмену последней транзакции, произведенной на устройстве.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция отмены последней транзакции прошла успешно, иначе `false`.
bool DriverPOSTerminal::EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"EmergencyReversal");

    return false;
}

// Метод: ПолучитьОперацииПоКартам (GetOperationByCards)
// Описание: 
// Выполняет получение отчета, содержащего список операций по картам, выполненных за текущий день.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ТаблицаОпераций (TableOperations) [OUT]  
//    XML таблица со списком операций по картам за день.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если отчет успешно сформирован, иначе `false`.
bool DriverPOSTerminal::GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"GetOperationByCards");

    return false;
}

// Метод: ИтогиДняПоКартам (Settlement)
// Описание: 
// Выполняет сверку итогов дня по операциям с картами. 
// Возвращает текст квитанции, сформированный эквайринговым ПО.
//
// Параметры:
// 0. STRING ИДУстройства (DeviceID) [IN]  
//    Идентификатор устройства.
//
// 1. STRING ТекстКвитанции (Slip) [OUT]  
//    Текст квитанции, сформированный эквайринговым ПО.  
//    * - Если информация передается из конфигурации "1С:Предприятия", содержание поля должно соответствовать требованиям конкретной эквайринговой системы.  
//    ** - Если необходимо сформировать несколько документов, текст должен содержать строку "[отрезка]" или "[cut]" для их разделения.  
//    Текст не должен содержать управляющие символы с кодами 1-8, 11-12, 14-31.
//
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция выполнена успешно, иначе `false`.
bool DriverPOSTerminal::Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"Settlement");

    return false;
}

bool DriverPOSTerminal::PrintSlipOnTerminal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 0, u"Settlement");

    return false;
}

const std::u16string DriverPOSTerminal::getEquipmentId()
{
    return m_equipmentId;
}

void DriverPOSTerminal::AddActionDriver(const std::u16string& name_en, const std::u16string& name_ru, const std::u16string& caption_en, const std::u16string& caption_ru, CallAsFunc1C ptr_method)
{
	m_actions.push_back({ name_en, name_ru, caption_en, caption_ru, ptr_method });
}

std::span<const ActionDriver> DriverPOSTerminal::getActions()
{
	return m_actions;
}

std::optional<TerminalConfig> DriverPOSTerminal::getTerminalConfig(std::wstring deviceID)
{
	auto it = m_configTerminals.find(deviceID);
	if ( it != m_configTerminals.end()) {
		return it->second;
	}
	return std::nullopt;
}

bool DriverPOSTerminal::InitConnection(std::wstring& deviceID, std::wstring &error)
{
	auto newDeviceID = generateGUID();
	auto paramTypeConnection = findParameterValue<std::wstring>(m_ParamConnection, L"ConnectionType");
    if (paramTypeConnection.has_value()) {
		error = L"Invalid connection type";
        return false;
    }

	auto paramHost = findParameterValue<std::wstring>(m_ParamConnection, L"Host");
	if (paramHost.has_value()) {
		error = L"Invalid host";
		return false;
	}

	auto paramPort = findParameterValue<int>(m_ParamConnection, L"Port");
	if (paramPort.has_value()) {
		error = L"Invalid port";
		return false;
	}

    auto connType = wstringToConnectionType(paramTypeConnection.value());
	if (!connType.has_value()) {
		error = L"Invalid connection type";
		return false;
	}

	auto connectionType = connType.value();
    auto connection = ConnectionFactory::create(connectionType);
    auto protocolChanel = ChannelProtocolFactory::create(m_protocolTerminal, std::move(connection));

	auto host = str_utils::to_string(paramHost.value());
	auto port = paramPort.value();

	if (!protocolChanel.get()->connect(host, port)) {
		error = L"Failed to connect";
		return false;
	}
    
	bool result = protocolChanel->isConnected();
    if (result) {
		deviceID = newDeviceID;
        m_connections.insert({ newDeviceID, std::move(protocolChanel) });
    }

	return result;
}

bool DriverPOSTerminal::testConnection()
{
    return testConnection(m_ParamConnection);
}

bool DriverPOSTerminal::testConnection(std::vector<DriverParameter>& paramConnection)
{
    auto paramTypeConnection = findParameterValue<std::wstring>(paramConnection, L"ConnectionType");
    if (!paramTypeConnection.has_value()) {
        addErrorDriver(u"Invalid connection type", L"InitConnection: Invalid connection type");
        return false;
    }

    auto paramHost = findParameterValue<std::wstring>(paramConnection, L"Host");
    if (!paramHost.has_value()) {
        addErrorDriver(u"Invalid host", L"InitConnection: Invalid host");
        return false;
    }

    auto paramPort = findParameterValue<int>(paramConnection, L"Port");
    if (!paramPort.has_value()) {
        addErrorDriver(u"Invalid port", L"InitConnection: Invalid port");
        return false;
    }

    auto connType = wstringToConnectionType(paramTypeConnection.value());
    if (!connType.has_value()) {
        addErrorDriver(u"Invalid connection type", L"InitConnection: Invalid connection type");
        return false;
    }

    auto connectionType = connType.value();
    std::unique_ptr<IConnection> connection = ConnectionFactory::create(connectionType);
    auto host = str_utils::to_string(paramHost.value());
    auto port = paramPort.value();

    if (!connection.get()->connect(host, port)) {
        addErrorDriver(u"Failed to connect", L"InitConnection: Failed to connect");
        return false;
    }

    bool result = connection->isConnected();
    if (result) {
        connection->disconnect();
    }
	connection.reset();

    return result;
}

void DriverPOSTerminal::addErrorDriver(const std::u16string& lastError, const std::wstring& logError)
{
	m_lastError = lastError;
    LOG_ERROR_ADD(L"DriverTerminal", logError);
}

void DriverPOSTerminal::clearError()
{
	m_lastError = u"";
}

std::u16string DriverPOSTerminal::createUID(const std::wstring& host, uint32_t port)
{
    std::hash<std::wstring> hash_fn;
    size_t hash_host = hash_fn(host);
    size_t hash_port = std::hash<uint32_t>{}(port);

    // Combine the hashes
    size_t combined_hash = hash_host ^ (hash_port << 1);

    // Convert the combined hash to a hexadecimal string
    std::wstringstream ss;
    ss << std::hex << std::setw(16) << std::setfill(L'0') << combined_hash;
    return wstringToU16string(ss.str());
}

std::optional<EquipmentTypeInfo> DriverPOSTerminal::getEquipmentTypeInfoFromVariant(tVariant* paParam)
{
    std::u16string equipmentType;
    if (paParam->vt == VTYPE_PWSTR) {
        equipmentType = std::u16string(paParam->pwstrVal);
    }
    else {
        addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
		return std::nullopt;
    }

	auto type = getEquipmentTypeInfo(equipmentType);
    if (type.has_value())
	{
		addErrorDriver(u"Invalid EquipmentType", L"EquipmentParameters: Invalid EquipmentType");
		return std::nullopt;
	}

	return type;
}
