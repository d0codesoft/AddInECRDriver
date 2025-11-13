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

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shellapi.h>
#elif defined(__APPLE__) || defined(__MACH__)
#include <sys/stat.h> // For struct stat and stat
#endif
#include "localization_manager.h"

#define CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, lValidCountParams, methodName) \
    if ((lSizeArray) != (lValidCountParams) || (lSizeArray!=0 && !(paParams))) { \
        auto slValidCountParams = str_utils::to_u16string(std::to_wstring(lValidCountParams)); \
        std::u16string msg = u"Method expects " + slValidCountParams + u" parameter(s)"; \
        addErrorDriver(msg, str_utils::to_wstring(methodName) + L": Method expects " + std::to_wstring(lValidCountParams) + L" parameter(s)"); \
        m_addInBase->setBoolValue(pvarRetValue, false); \
        return false; \
    } \

//---------------------------------------------------------------------------//
DriverPOSTerminal::DriverPOSTerminal(IAddInBase * addInBase) : m_addInBase(addInBase)
{
	m_licenseManager = std::make_unique<LicenseManager>();
}

DriverPOSTerminal::~DriverPOSTerminal()
{
	if (m_licenseManager)
		m_licenseManager.reset();

	if (m_controller.size() > 0) {
		for (auto& connection : m_controller) {
			if (connection.second->isConnected()) {
                connection.second->disconnect();
			}
		}
	}
	m_controller.clear();
}

void DriverPOSTerminal::InitDriver()
{
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
		Logger::getLogFilePath(),
        LoadStringResource(L"IDS_DRIVER_NAME_ADDIN")
    };

	//OptionDriverNames
	//L"ConnectionType"
    m_ParamConnection = {
        { OptionDriverNames.at(DriverOption::ConnectionType), static_cast<int>(ConnectionType::TCP), TypeParameter::String},
	    { OptionDriverNames.at(DriverOption::Address), L"", TypeParameter::String },
	    { OptionDriverNames.at(DriverOption::Port), 2000, TypeParameter::Number },
		{ OptionDriverNames.at(DriverOption::Speed), 9600, TypeParameter::Number },
		{ OptionDriverNames.at(DriverOption::LogLevel), static_cast<int>(LogLevel::Debug), TypeParameter::String },
		{ OptionDriverNames.at(DriverOption::MerchantId), 0, TypeParameter::Number },
		{ OptionDriverNames.at(DriverOption::Facepay), false, TypeParameter::Bool },
		{ OptionDriverNames.at(DriverOption::LogFullPath), Logger::getLogFilePath(), TypeParameter::String }
	};

	AddActionDriver(L"OpenLogFile", L"ОткрытьФайлЛога", L"Open log file", L"Відкрити файл лога", 
		std::bind(&DriverPOSTerminal::ActionOpenFileLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	AddActionDriver(L"OpenSettings", L"OpenSettings", L"Open log file", L"Налаштування драйверу",
		std::bind(&DriverPOSTerminal::ActionOpenConfiguration, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

    // 2000 revision is the minimum supported revision 2.0
    // 2004 revision is the maximum supported revision 2.4
	// 3004 revision is the maximum supported revision 2.4
    // 4004 revision is the maximum supported revision 4.4

    tVarInit(pvarRetValue);
    TV_VT(pvarRetValue) = VTYPE_INT;
	TV_INT(pvarRetValue) = DRIVER_REQUIREMENTS_VERSION;

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

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetLastError");

    m_addInBase->setStringValue(paParams, m_lastError);
	m_addInBase->setBoolValue(pvarRetValue, true);
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
	clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"GetParameters");

	auto xmlParam = toXML(m_settingDriverPos.getSettings(), this->m_ParamConnection);
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
	clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"SetParameter");

	auto optParam = VariantHelper::getStringValue(paParams[0]);
    if (!optParam.has_value()) {
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}
	std::wstring paramName = optParam.value();

    if (VariantHelper::isValueString(paParams[1])) {
        auto valStr = VariantHelper::getStringValue(paParams[1]);
		if (valStr.has_value()) {
            setParameterValue(m_ParamConnection, paramName, valStr.value());
        }
	}
	else if (VariantHelper::isValueLong(paParams[1])) {
        auto valLong = VariantHelper::getLongValue(paParams[1]);
        if (valLong.has_value()) {
            setParameterValue(m_ParamConnection, paramName, valLong.value());
        }
	}
	else if (VariantHelper::isValueBool(paParams[1])) {
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
	clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"Open");

    std::wstring error = {}, deviceId = {};
    auto result = InitConnection(deviceId, error);
    if (result)
    {
        m_addInBase->setStringValue(paParams, str_utils::to_u16string(deviceId));
		// Need initialize configuration terminal
		// And get parameters from terminal
		_initializeConfigurationTerminal(deviceId);
    }
    else {
		_handleError(L"Open", error);
        TV_VT(paParams) = VTYPE_EMPTY;
    }

    m_addInBase->setBoolValue(pvarRetValue, result);

    return result;
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
	clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 1, u"Close");

    std::wstring deviceId{};
    auto _connect = getDeviceConnection(paParams, deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"Close", L"Invalid type for device ID");
    }
    
    if (_connect->get()->isConnected())
    {
        _connect->get()->disconnect();
    }
    _connect->get().reset();
	m_controller.erase(deviceId);

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
	clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"DeviceTest");

	// Test connection
	auto result = testConnection(m_ParamConnection);

	m_addInBase->setStringValue(&paParams[0], str_utils::to_u16string(m_driverDescription.Description));

	if (m_licenseManager->isDemoMode()) {
        m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(m_licenseManager->getDemoModeDescription()));
	}
	else {
		m_addInBase->setStringValue(&paParams[1], u"");
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
		return fail(pvarRetValue, L"EquipmentParameters", L"Invalid parameter EquipmentType");
	}

    // Get SettingXML
	auto settingXml = m_settingDriverPos.getSettingXML();
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
		return fail(pvarRetValue, L"ConnectEquipment", L"Invalid parameter EquipmentType");
    }

    // Get ConnectionParameters (STRING[IN])
	auto paramConnect = VariantHelper::getStringValue(paParams[1]);
    if (!paramConnect.has_value()) {
		return fail(pvarRetValue, L"ConnectEquipment", L"Invalid type for ConnectionParameters");
	}

    // Parce xml parametrs
	if (!ParseParametersFromXML(m_ParamConnection, paramConnect.value())) {
		return fail(pvarRetValue, L"ConnectEquipment", L"Invalid ConnectionParameters");
	}

	std::wstring error, deviceId;

    auto result = InitConnection(deviceId, error);
	if (result)
	{
        m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(deviceId));
	}
    else {
		_handleError(L"ConnectEquipment", error);
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
		return fail(pvarRetValue, L"DisconnectEquipment", L"Invalid type for device ID");
	}

	auto findId = m_controller.find(deviceID);
	if (findId == m_controller.end()) {
		return fail(pvarRetValue, L"DisconnectEquipment", L"Invalid device ID");
	}

	if (findId->second != nullptr)
	{
		if (findId->second.get()->isConnected())
		{
            findId->second.get()->disconnect();
		}
        findId->second.reset();
	}
	m_controller.erase(findId);
    
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
		return fail(pvarRetValue, L"EquipmentTest", L"Invalid parameter EquipmentType");
    }

	// Get ConnectionParameters (STRING[IN])
	auto connectionParameters = VariantHelper::getStringValue(paParams[1]);
    if (!connectionParameters.has_value()) {
		return fail(pvarRetValue, L"EquipmentTest", L"Invalid type for ConnectionParameters");
    }

	std::vector<DriverParameter> paramConnection;
    if (!ParseParametersFromXML(paramConnection, connectionParameters.value())) {
		return fail(pvarRetValue, L"EquipmentTest", L"Invalid ConnectionParameters");
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
		return fail(pvarRetValue, L"EquipmentTest", L"Error connection");
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

    return result;
}

//********************************************************************************************************************
// Метод: ВыполнитьДополнительноеДействие (DoAdditionalAction)
// Описание: 
// Выполняет дополнительное действие, указанное по имени.  
// Действие должно быть предварительно определено в списке, полученном методом `ПолучитьДополнительныеДействия`.
//
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
	auto optVal = VariantHelper::getStringValue(paParams[0]);
	if (!optVal.has_value()) {
		return fail(pvarRetValue, L"DoAdditionalAction", L"Invalid type for ActionName");
	}

	auto actionName = optVal.value();
	auto find_action = std::find_if(actions.begin(), actions.end(), [&](const ActionDriver& action) {
		return actionName == action.name_en || actionName == action.name_ru;
		});

	if (find_action == actions.end()) {
		return fail(pvarRetValue, L"DoAdditionalAction", L"Action not found");
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
    return m_MethodNames;
}

const std::vector<PropName>& DriverPOSTerminal::GetProperties() {
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

	auto optDeviceId = VariantHelper::getStringValue(paParams[0]);
	if (!optDeviceId.has_value()) {
		return fail(pvarRetValue, L"TerminalParameters", L"Invalid type for DeviceID");
	}

	auto deviceId = optDeviceId.value();
    auto param = getTerminalConfig(deviceId);
	if (!param) {
		return fail(pvarRetValue, L"TerminalParameters", L"Invalid device ID");
	}

    auto xmlParameter = toXMLTerminalConfig(*param);
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
    
	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"Pay", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"Pay", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"Pay", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"Pay", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::Pay;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"Pay", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"Pay", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"Pay", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"Pay", L"Invalid OperationParameters format");
	}
	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));

	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	// Check the number of parameters
#if DRIVER_REQUIREMENTS_VERSION<=DRIVER_REQUIREMENTS_VERSION_3_4
	CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 7, u"PayByPaymentCard");
#elif DRIVER_REQUIREMENTS_VERSION<DRIVER_REQUIREMENTS_VERSION_4_1
	CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 8, u"PayByPaymentCard");
#esle
	CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 9, u"PayByPaymentCard");
#endif

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"PayByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"PayByPaymentCard", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"PayByPaymentCard", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"PayByPaymentCard", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::Pay;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"PayByPaymentCard", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"PayByPaymentCard", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(resultPay->pan));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(resultPay->invoiceNumber));
	m_addInBase->setStringValue(&paParams[4], str_utils::to_u16string(resultPay->rrn));
	m_addInBase->setStringValue(&paParams[5], str_utils::to_u16string(resultPay->approvalCode));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(resultPay->receipt));
	m_addInBase->setBoolValue(pvarRetValue, resultPay->result);

    return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::ReturnPayment;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(resultPay->pan));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(resultPay->invoiceNumber));
	m_addInBase->setStringValue(&paParams[4], str_utils::to_u16string(resultPay->rrn));
	m_addInBase->setStringValue(&paParams[5], str_utils::to_u16string(resultPay->approvalCode));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(resultPay->receipt));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"ReturnPayment", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"ReturnPayment", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"ReturnPayment", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"ReturnPayment", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::ReturnPayment;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"ReturnPayment", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"ReturnPayment", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"ReturnPayment", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"ReturnPayment", L"Invalid OperationParameters format");
	}

	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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
    
	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"CancelPayment", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"CancelPayment", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"CancelPayment", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"CancelPayment", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::CancelPayment;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"CancelPayment", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"CancelPayment", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"CancelPayment", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"CancelPayment", L"Invalid OperationParameters format");
	}

	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"CancelPaymentByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);
	auto RRNCode = VariantHelper::getStringValue(paParams[4]);
	auto AuthorizationCode = VariantHelper::getStringValue(paParams[5]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::CancelPayment;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}
	if (RRNCode.has_value()) {
		paramPayment.RRNCode = RRNCode.value();
	}
	if (AuthorizationCode.has_value()) {
		paramPayment.AuthorizationCode = AuthorizationCode.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(paramPayment.CardNumber));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.ReceiptNumber));
	m_addInBase->setStringValue(&paParams[4], str_utils::to_u16string(paramPayment.RRNCode));
	m_addInBase->setStringValue(&paParams[5], str_utils::to_u16string(paramPayment.AuthorizationCode));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"CancelPaymentByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"Authorisation", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"Authorisation", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"Authorisation", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::Authorisation;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"Authorisation", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"Authorisation", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"Authorisation", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"Authorisation", L"Invalid OperationParameters format");
	}

	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"AuthorisationByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"AuthorisationByPaymentCard", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"AuthorisationByPaymentCard", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"AuthorisationByPaymentCard", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::Authorisation;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"AuthorisationByPaymentCard", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"AuthorisationByPaymentCard", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(paramPayment.CardNumber));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.ReceiptNumber));
	m_addInBase->setStringValue(&paParams[4], str_utils::to_u16string(paramPayment.RRNCode));
	m_addInBase->setStringValue(&paParams[5], str_utils::to_u16string(paramPayment.AuthorizationCode));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::AuthConfirmation;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"AuthConfirmation", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"AuthConfirmation", L"Invalid OperationParameters format");
	}

	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

    return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"AuthConfirmationByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"AuthConfirmationByPaymentCard", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"AuthConfirmationByPaymentCard", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"AuthConfirmationByPaymentCard", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::AuthConfirmation;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"AuthConfirmationByPaymentCard", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"AuthConfirmationByPaymentCard", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(paramPayment.CardNumber));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.ReceiptNumber));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::CancelAuthorisation;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Invalid parameters for cancel authorization");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"CancelAuthorisation", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"CancelAuthorisation", L"Invalid OperationParameters format");
	}

	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"CancelAuthorisationByPaymentCard", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);
	auto RRNCode = VariantHelper::getStringValue(paParams[4]);
	auto AuthorizationCode = VariantHelper::getStringValue(paParams[5]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::CancelAuthorisation;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}
	if (RRNCode.has_value()) {
		paramPayment.RRNCode = RRNCode.value();
	}
	if (AuthorizationCode.has_value()) {
		paramPayment.AuthorizationCode = AuthorizationCode.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"ReturnPaymentByPaymentCard", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[1], str_utils::to_u16string(paramPayment.CardNumber));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.ReceiptNumber));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Device is not connected");
	}

	if (!VariantHelper::isValueString(paParams[1])) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Invalid type for OperationParameters");
	}

	auto _paramXML = VariantHelper::getStringValue(paParams[1]);
	if (!_paramXML.has_value()) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Missing OperationParameters");
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::PayWithCashWithdrawal;
	if (!readPOSTerminalOperationParametersFromXml(_paramXML.value(), paramPayment)) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Invalid OperationParameters format");
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", _errorCode);
	}

	std::wstring _resultXML{};
	if (!writePOSTerminalResponseToXml(resultPay, paramPayment.OperationType, _resultXML)) {
		return fail(pvarRetValue, L"PayWithCashWithdrawal", L"Invalid OperationParameters format");
	}

	m_addInBase->setStringValue(&paParams[2], str_utils::to_u16string(_resultXML));
	m_addInBase->setStringValue(&paParams[3], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);
    return true;
}

// >= DRIVER_REQUIREMENTS_VERSION_4_1 <=DRIVER_REQUIREMENTS_VERSION_4_3
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"PayByPaymentCardWithCashWithdrawal", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	if (!connection.get()->isConnected()) {
		return fail(pvarRetValue, L"PayByPaymentCardWithCashWithdrawal", L"Device is not connected");
	}

	auto ammount = VariantHelper::getDoubleValue(paParams[2]);
	if (!ammount.has_value()) {
		return fail(pvarRetValue, L"PayByPaymentCardWithCashWithdrawal", L"Invalid type for Amount");
	}

	auto paramMerchantId = findParameterValue<long>(m_ParamConnection, DriverOption::MerchantId);
	if (!paramMerchantId.has_value()) {
		return fail(pvarRetValue, L"PayByPaymentCardWithCashWithdrawal", L"Merchant id parameter is not filled");
	}

	bool _facePay = false;
	auto paramFacepay = findParameterValue<bool>(m_ParamConnection, DriverOption::Facepay);
	if (paramFacepay.has_value()) {
		_facePay = paramFacepay.value();
	}

	auto CardNumber = VariantHelper::getStringValue(paParams[1]);
	auto ReceiptNumber = VariantHelper::getStringValue(paParams[3]);
	auto RRNCode = VariantHelper::getStringValue(paParams[4]);
	auto AuthorizationCode = VariantHelper::getStringValue(paParams[5]);

	long _merchantId = 0;
	if (paramMerchantId.has_value()) {
		_merchantId = paramMerchantId.value();
	}

	POSTerminalOperationParameters paramPayment;
	paramPayment.OperationType = POSTerminalOperationType::PayWithCashWithdrawal;
	paramPayment.Amount = ammount.value();
	paramPayment.MerchantNumber = _merchantId;
	if (CardNumber.has_value()) {
		paramPayment.CardNumber = CardNumber.value();
	}
	paramPayment.FacePay(_facePay);
	if (ReceiptNumber.has_value()) {
		paramPayment.ReceiptNumber = ReceiptNumber.value();
	}
	if (RRNCode.has_value()) {
		paramPayment.RRNCode = RRNCode.value();
	}
	if (AuthorizationCode.has_value()) {
		paramPayment.AuthorizationCode = AuthorizationCode.value();
	}

	if (!isValidPOSTerminalOperationParameters(paramPayment)) {
		return fail(pvarRetValue, L"PayByPaymentCardWithCashWithdrawal", L"Invalid parameters for payment");
	}

	std::wstring _errorCode{};
	auto resultPay = connection.get()->processTransaction(paramPayment, _errorCode);
	if (!resultPay) {
		return fail(pvarRetValue, L"PayByPaymentCardWithCashWithdrawal", _errorCode);
	}

	m_addInBase->setStringValue(&paParams[5], str_utils::to_u16string(paramPayment.CardNumber));
	m_addInBase->setStringValue(&paParams[6], str_utils::to_u16string(paramPayment.ReceiptNumber));
	m_addInBase->setStringValue(&paParams[7], str_utils::to_u16string(paramPayment.RRNCode));
	m_addInBase->setStringValue(&paParams[8], str_utils::to_u16string(paramPayment.AuthorizationCode));
	m_addInBase->setStringValue(&paParams[9], str_utils::to_u16string(paramPayment.Slip));
	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"PurchaseWithEnrollment", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	return fail(pvarRetValue, L"PurchaseWithEnrollment", L"Operation not supported");
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"GetCardParametrs", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	return fail(pvarRetValue, L"GetCardParametrs", L"Operation not supported");
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"PayCertificate", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	return fail(pvarRetValue, L"PayCertificate", L"Operation not supported");
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"ReturnCertificate", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

	return fail(pvarRetValue, L"ReturnCertificate", L"Operation not supported");
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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(paParams, deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"EmergencyReversal", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"GetOperationByCards", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

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

	std::wstring deviceId{};
	auto _connect = getDeviceConnection(&paParams[0], deviceId);
	if (!_connect.has_value()) {
		return fail(pvarRetValue, L"Settlement", L"Invalid type for DeviceID");
	}

	auto& connection = _connect.value();

    return false;
}

// Метод: ПечатьКвитанцийНаТерминале (PrintSlipOnTerminal)
// Описание: 
// Возвращает будетли терминал самостоятельно печатать квитанции на своем принтере для операций
//
// Параметры: Нет
//		
// Возвращаемое значение:
// BOOL  
// Возвращает `true`, если операция выполнена успешно, иначе `false`.
bool DriverPOSTerminal::PrintSlipOnTerminal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    clearError();
    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 0, u"PrintSlipOnTerminal");

	m_addInBase->setBoolValue(pvarRetValue, true);

    return true;
}

void DriverPOSTerminal::AddActionDriver(const std::wstring& name_en, const std::wstring& name_ru, const std::wstring& caption_en, const std::wstring& caption_ru, CallAsFunc1C ptr_method)
{
	m_actions.push_back({ name_en, name_ru, caption_en, caption_ru, ptr_method });
}

std::span<const ActionDriver> DriverPOSTerminal::getActions()
{
	return m_actions;
}

POSTerminalConfig* DriverPOSTerminal::getTerminalConfig(const std::wstring& deviceID)
{
	auto it = m_configTerminals.find(deviceID);
	if ( it != m_configTerminals.end()) {
		return &it->second;
	}
	return nullptr;
}

bool DriverPOSTerminal::ActionOpenFileLog(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	clearError();

	// Get the log file path
	std::wstring logFilePath = Logger::getLogFilePath();

	// Check if the file exists
	if (logFilePath.empty() || GetFileAttributes(logFilePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
		return fail(pvarRetValue, L"ActionOpenFileLog", L"Invalid type for DeviceID");
	}

#if defined(CURRENT_OS_WINDOWS)

	// Open the file using the default associated application
	HINSTANCE result = ShellExecute(
		nullptr,                // Parent window handle (nullptr for no parent)
		L"open",                // Operation to perform
		logFilePath.c_str(),    // File to open
		nullptr,                // Parameters (not needed for opening a file)
		nullptr,                // Default directory (not needed)
		SW_SHOWNORMAL           // Show the application normally
	);

	// Check if the operation was successful
	if (reinterpret_cast<std::intptr_t>(result) <= 32) {
		return fail(pvarRetValue, L"ActionOpenFileLog", L"Failed to open log file");
	}
#elif defined(CURRENT_OS_MACOS)
	// macOS-specific implementation
	std::string command = "open \"" + str_utils::to_string(logFilePath) + "\"";
	if (std::system(command.c_str()) != 0) {
		return fail(pvarRetValue, L"ActionOpenFileLog", L"Failed to open log file");
	}
#elif defined(CURRENT_OS_LINUX)
	// Linux-specific implementation
	std::string command = "xdg-open \"" + str_utils::to_string(logFilePath) + "\"";
	if (std::system(command.c_str()) != 0) {
		return fail(pvarRetValue, L"ActionOpenFileLog", L"Failed to open log file");
	}
#else
	// Unsupported platform
	return fail(pvarRetValue, L"ActionOpenFileLog", L"Unsupported platform");
#endif

	m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
}

bool DriverPOSTerminal::ActionOpenConfiguration(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	clearError();

	// Open the configuration dialog
	//if (m_showConfigDialog) {
	//	m_showConfigDialog();
	//	return true;
	//}
	
	m_addInBase->setBoolValue(pvarRetValue, true);
	return true;
}

bool DriverPOSTerminal::InitConnection(std::wstring& deviceID, std::wstring &error)
{
	auto newDeviceID = generateGUID();
	auto paramTypeConnection = findParameterValue<long>(m_ParamConnection, DriverOption::ConnectionType);
    if (!paramTypeConnection.has_value()) {
		error = L"Invalid connection type";
        return false;
    }

	auto paramHost = findParameterValue<std::wstring>(m_ParamConnection, DriverOption::Address);
	if (!paramHost.has_value()) {
		error = L"Invalid host";
		return false;
	}

	auto paramPort = findParameterValue<long>(m_ParamConnection, DriverOption::Port);
	if (!paramPort.has_value()) {
		error = L"Invalid port";
		return false;
	}

    auto connType = toConnectionType(paramTypeConnection.value());
	if (!connType.has_value()) {
		error = L"Invalid connection type";
		return false;
	}

	auto connectionType = connType.value();
    auto terminalController = POSTerminalControllerFactory::create(m_protocolTerminal, connectionType);

	auto host = str_utils::to_string(paramHost.value());
	auto port = static_cast<uint16_t>(paramPort.value());

	if (!terminalController.get()->connect(host, port)) {
		error = L"Failed to connect";
		return false;
	}
    
	bool result = terminalController->isConnected();
    if (result) {
		deviceID = newDeviceID;
		m_controller.insert({ newDeviceID, std::move(terminalController) });
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

    auto paramPort = findParameterValue<long>(paramConnection, L"Port");
    if (!paramPort.has_value()) {
        addErrorDriver(u"Invalid port", L"InitConnection: Invalid port");
        return false;
    }

    auto connType = toConnectionType(paramTypeConnection.value());
    if (!connType.has_value()) {
        addErrorDriver(u"Invalid connection type", L"InitConnection: Invalid connection type");
        return false;
    }

    auto connectionType = connType.value();
	auto protocolChanel = POSTerminalControllerFactory::create(m_protocolTerminal, connectionType);

	auto host = str_utils::to_string(paramHost.value());
	auto port = static_cast<uint16_t>(paramPort.value());

	if (!protocolChanel.get()->connect(host, port)) {
		addErrorDriver(u"Failed to connect", L"InitConnection: Failed to connect");
		return false;
	}

    bool result = protocolChanel->isConnected();
    if (result) {
        protocolChanel->disconnect();
    }
    protocolChanel.reset();

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
    return str_utils::to_u16string(ss.str());
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

std::optional<std::reference_wrapper<std::unique_ptr<POSTerminalController>>> DriverPOSTerminal::getDeviceConnection(tVariant* paramDeviceID, std::wstring& deviceId)
{
	deviceId = L"";
	if (VariantHelper::isValueString(*paramDeviceID)) {
		auto opVal = VariantHelper::getStringValue(*paramDeviceID);
		if (opVal.has_value()) {
            deviceId = opVal.value();
			auto findId = m_controller.find(deviceId);
			if (findId != m_controller.end() && findId->second != nullptr) {
				return std::optional<std::reference_wrapper<std::unique_ptr<POSTerminalController>>>(findId->second);
			}
		}
	}
	return std::nullopt;
}

void DriverPOSTerminal::_handleError(const std::wstring& methodName, const std::wstring& messageError, const bool driverErrorNotify, const AddinErrorCode errorCode)
{
	auto error_ = str_utils::to_u16string(messageError);
	if (driverErrorNotify) {
		m_addInBase->addError(static_cast<int>(errorCode), str_utils::to_u16string(methodName), error_, -1);
	}
	auto log_error = methodName + L": " + messageError;
	addErrorDriver(error_, log_error);
}

bool DriverPOSTerminal::fail(tVariant* pvarRetValue, const std::wstring& context, const std::wstring& errorKey)
{
	_handleError(context, LocalizationManager::GetLocalizedString(errorKey));
	m_addInBase->setBoolValue(pvarRetValue, false);
	return false;
}

void DriverPOSTerminal::_initializeConfigurationTerminal(const std::wstring& id)
{
	auto config = this->getTerminalConfig(id);
	if (config == nullptr) {
		m_configTerminals[id] = POSTerminalConfig{};
		config = &m_configTerminals[id];
	}

	// Initialize settings
	config->CashWithdrawal = false;
	config->ConsumerPresentedQR = false;
	config->ElectronicCertificates = false;
	config->ListCardTransactions = false;
	config->PartialCancellation = false;
	config->PrintSlipOnTerminal = true;
	config->PurchaseWithEnrollment = false;
	config->ReturnElectronicCertificateByBasketID = false;
	config->ShortSlip = false;
	config->TerminalID = id;
	config->TerminalModel = m_controller[id]->getTerminalModel();
	config->TerminalVendor = m_controller[id]->getTerminalVendor();

	if (config->TerminalVendor == L"PAX")
	{

	}
}
