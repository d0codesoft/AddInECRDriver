#include "pch.h"
#include <iomanip>
#include <string>
#include <functional>
#include <sstream>
#include "driver_pos_terminal.h"
#include "interface_pos_terminal.h"
#include "string_conversion.h"
#include "sys_utils.h"
#include "setting_driver_pos.h"
#include "logger.h"

#define CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, lValidCountParams, methodName) \
    if ((lSizeArray) != (lValidCountParams) || !(paParams)) { \
        auto slValidCountParams = wstringToU16string(std::to_wstring(lValidCountParams)); \
        std::u16string msg = u"Method expects " + slValidCountParams + u" parameter(s)"; \
        addErrorDriver(msg, u16stringToWstring(methodName) + L": Method expects " + std::to_wstring(lValidCountParams) + L" parameter(s)"); \
        m_addInBase->setBoolValue(pvarRetValue, false); \
        return false; \
    } \


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
    // 4004 revision is the maximum supported revision 4.4

    tVarInit(pvarRetValue);
    TV_VT(pvarRetValue) = VTYPE_I4;
    TV_I4(pvarRetValue) = 4004;

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
    // Ensure no parameters are expected
    if (lSizeArray != 0 || !paParams)
    {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"GetDescription", u"Method does not accept parameters", -1);
        addErrorDriver(u"Method GetDescription does not accept parameters", L"GetDescription: Method does not accept parameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    // Convert the structure to a JSON string
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
    return false;
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
    return false;
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
    return false;
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
    return false;
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

    CHECK_PARAMS_COUNT(pvarRetValue, paParams, lSizeArray, 2, u"GetInterfaceRevision");

	if (lSizeArray != 2 || !paParams)
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Method expects two parameter", -1);
		addErrorDriver(u"Method EquipmentParameters expects two parameter", L"EquipmentParameters: Method expects two parameter");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}


	auto type = getEquipmentTypeInfoFromVariant(&paParams[1]);
	if (!type.has_value() || type.value() != EquipmentTypeInfo::POSTerminal) {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid type for EquipmentType", -1);
		addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

    // Get SettingXML
	auto settingXml = SettingDriverPos::getSettingXML();
	m_addInBase->setStringValue(paParams, settingXml);
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
	if (lSizeArray != 3 || !paParams)
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Method expects three parameter", -1);
        addErrorDriver(u"Method ConnectEquipment expects three parameter", L"ConnectEquipment: Method expects three parameter");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
	}
    
    // Get EquipmentType (STRING[IN])
    auto type = getEquipmentTypeInfoFromVariant(&paParams[1]);
    if (!type.has_value() || type.value() != EquipmentTypeInfo::POSTerminal) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid type for EquipmentType", -1);
        addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    // Get ConnectionParameters (STRING[IN])
    std::u16string connectionParameters;
    if (paParams[2].vt == VTYPE_PWSTR) {
        connectionParameters = paParams[2].pwstrVal;
    }
    else {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Invalid type for ConnectionParameters", -1);
        addErrorDriver(u"Invalid type for ConnectionParameters", L"ConnectEquipment: Invalid type for ConnectionParameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    // Parce xml parametrs
	auto paramConnect = ParseParameters(u16stringToWstring(connectionParameters));
	if (paramConnect.empty())
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Invalid ConnectionParameters", -1);
        addErrorDriver(u"Invalid ConnectionParameters", L"ConnectEquipment: Invalid ConnectionParameters");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
	}
	// Connect to equipment
	m_ParamConnection = paramConnect;
    auto result = InitConnection();
	if (result)
	{
		m_addInBase->setStringValue(&paParams[0], getEquipmentId());
	}
    else {
        TV_VT(&paParams[0]) = VTYPE_EMPTY;
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

    std::u16string equipmentId;
    if (paParams->vt == VTYPE_PWSTR) {
        equipmentId = std::u16string(paParams->pwstrVal);
	}
	else {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"DisconnectEquipment", u"Invalid type for equipmentId", -1);
		addErrorDriver(u"Invalid type for equipmentId", L"DisconnectEquipment: Invalid type for equipmentId");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
	}

    if (equipmentId != m_equipmentId) {
		addErrorDriver(u"Invalid equipmentId", L"DisconnectEquipment: Invalid equipmentId");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

	if (m_connection != nullptr)
	{
		if (m_connection.get()->isConnected())
		{
			m_connection.get()->disconnect();
		}
		m_connection.reset();
	}

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



   // Get EquipmentType (STRING[IN])
    auto type = getEquipmentTypeInfoFromVariant(&paParams[1]);
    if (!type.has_value() || type.value() != EquipmentTypeInfo::POSTerminal) {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid type for EquipmentType", -1);
        addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

    return false;
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
    return false;
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
    return false;
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
    return false;
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
    return false;
}

const std::u16string DriverPOSTerminal::getEquipmentId()
{
    return m_equipmentId;
}

bool DriverPOSTerminal::InitConnection()
{
	if (m_connection != nullptr)
	{
		if (m_connection.get()->isConnected())
		{
            m_connection.get()->disconnect();
		}
	}

	auto paramTypeConnection = findParameterValue(m_ParamConnection, L"ConnectionType");
    if (paramTypeConnection.has_value()) {
        addErrorDriver(u"Invalid connection type", L"InitConnection: Invalid connection type");
        return false;
    }

	auto paramHost = findParameterValue(m_ParamConnection, L"Host");
	if (paramHost.has_value()) {
		addErrorDriver(u"Invalid host", L"InitConnection: Invalid host");
		return false;
	}

	auto paramPort = findParameterValue(m_ParamConnection, L"Port");
	if (paramPort.has_value()) {
		addErrorDriver(u"Invalid port", L"InitConnection: Invalid port");
		return false;
	}

    auto connType = wstringToConnectionType(paramTypeConnection.value());
	if (!connType.has_value()) {
		addErrorDriver(u"Invalid connection type", L"InitConnection: Invalid connection type");
		return false;
	}

	m_connectionType = connType.value();
    m_connection = ConnectionFactory::create(m_connectionType);
	auto host = convertWStringToString(paramHost.value());
	auto port = wstringToUint16(paramPort.value());

	if (!m_connection.get()->connect(host, port)) {
		addErrorDriver(u"Failed to connect", L"InitConnection: Failed to connect");
		return false;
	}
	m_equipmentId = createUID(paramHost.value(), port);

	return m_connection->isConnected();
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
    ss << std::hex << std::setw(16) << std::setfill('0') << combined_hash;
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
