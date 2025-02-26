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

bool DriverPOSTerminal::GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    m_addInBase->setBoolValue(pvarRetValue, true);
    m_addInBase->setStringValue(paParams, m_lastError);
    return true;
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
// 📄 Описание: 
// Отключает оборудование, используя переданный идентификатор устройства.
// Освобождает ресурсы, связанные с подключенным экземпляром устройства.
//
// 🏷 Имя процедуры: ОтключитьОборудование (DisconnectEquipment)
//
// 📥 Параметры:
//   1) Тип: std::wstring   // Идентификатор устройства
//      Имя: идУстройства
//      Направление: [IN]
//      Описание: Принимает уникальный идентификатор устройства, 
//                который был возвращён при подключении.
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true — если оборудование успешно отключено;
//       false — если отключение выполнить не удалось (например, 
//               при неверном идентификаторе устройства или если устройство уже отключено).
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
// 📄 Описание: 
// ТестированиеОборудования (EquipmentTest)
//
// Выполняет пробное подключение и опрос устройства с текущими значениями параметров, 
// установленными функцией «УстановитьПараметр». 
// При успешном выполнении возвращает информацию об устройстве в описании.
// Если активирован демонстрационный режим, возвращает описание его ограничений.
//
// 🏷 Имя процедуры: ТестированиеОборудования (EquipmentTest)
//
// 📥 Параметры:
//   1) Тип: std::wstring   // Тип оборудования
//      Имя: типОборудования
//      Направление: [IN]
//      Описание: Имеет одно из значений из таблицы "Типоборудования". 
//                Определяет тип тестируемого устройства (например, "ККТ", "СканерШтрихкода").
//
//   2) Тип: std::wstring   // Параметры подключения в XML-формате
//      Имя: параметрыПодключения
//      Направление: [IN]
//      Описание: XML-таблица с параметрами подключения, используемыми для тестового взаимодействия с устройством.
//
// 📤 Выходные параметры:
//   3) Тип: std::wstring   // Описание результата
//      Имя: описание
//      Направление: [OUT]
//      Описание: Возвращает описание результата теста. В случае успеха содержит информацию об устройстве,
//                в противном случае — сообщение об ошибке.
//
//   4) Тип: std::wstring   // Описание демо-режима
//      Имя: активированДемоРежим
//      Направление: [OUT]
//      Описание: Содержит описание ограничений демонстрационного режима, если он активирован. 
//                В противном случае возвращает пустую строку. Пример: "Для полноценной работы требуется ключ защиты."
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true — если тестирование прошло успешно и устройство доступно для использования;
//       false — при возникновении ошибок подключения или несоответствия параметров.
//
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
// 📄 Описание: 
// АвтонастройкаОборудования (EquipmentAutoSetup)
//
// Выполняет автоматическую настройку оборудования. 
// В процессе настройки драйвер может отобразить технологическое окно для взаимодействия с пользователем.
// При успешном завершении возвращает обновленные параметры подключения оборудования.
//
// 🏷 Имя процедуры: АвтонастройкаОборудования (EquipmentAutoSetup)
//
// 📥 Входные параметры:
//   1) Тип: std::wstring   // Параметры подключения (по умолчанию)
//      Имя: параметрыПодключенияПоУмолчанию
//      Направление: [IN]
//      Описание: XML-таблица с параметрами подключения оборудования по умолчанию, 
//                которые будут использоваться при попытке авто-настройки.
//
//   2) Тип: bool            // Флаг отображения окна авто-настройки
//      Имя: показыватьОкноАвтонастройки
//      Направление: [IN]
//      Описание: Если true — драйвер отображает технологическое окно при авто-настройке. 
//                Если false — процесс происходит в фоновом режиме без интерфейса.
//
//   3) Тип: long            // Таймаут операции
//      Имя: таймаут
//      Направление: [IN]
//      Описание: Время в миллисекундах, по истечении которого операция будет прервана. 
//                Значение 0 означает отсутствие ограничения по времени.
//
// 📤 Выходные параметры:
//   4) Тип: std::wstring    // Тип оборудования
//      Имя: типОборудования
//      Направление: [OUT]
//      Описание: Строка с типом оборудования. Имеет одно из значений, указанных в таблице "Типоборудования".
//
//   5) Тип: std::wstring    // Обновленные параметры подключения
//      Имя: параметрыПодключенияНастроенные
//      Направление: [OUT]
//      Описание: XML-таблица с параметрами подключения оборудования, полученными в результате успешной авто-настройки.
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true — если авто-настройка выполнена успешно и параметры подключения обновлены;
//       false — при возникновении ошибок или прерывании операции (например, по тайм-ауту).
//
//********************************************************************************************************************
bool DriverPOSTerminal::EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

//********************************************************************************************************************
// 📄 Описание: 
// УстановитьИнформациюПриложения (SetApplicationInformation)
//
// Метод передает в драйвер информацию о приложении, в котором используется данный драйвер.
// Информация может включать версию приложения, идентификатор, название или другие параметры,
// необходимые для корректной интеграции и работы драйвера.
//
// 🏷 Имя процедуры: УстановитьИнформациюПриложения (SetApplicationInformation)
//
// 📥 Входные параметры:
//   1) Тип: std::wstring
//      Имя: параметрыПриложения
//      Направление: [IN]
//      Описание: XML-строка с параметрами приложения. 
//                Может содержать следующие элементы (пример):
//                <ApplicationSettings>
//                    <AppName>POSSystem</AppName>
//                    <AppVersion>2.5.1</AppVersion>
//                    <AppID>12345</AppID>
//                </ApplicationSettings>
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true — если информация о приложении успешно обработана драйвером;
//       false — при возникновении ошибок обработки (например, некорректный формат XML).
//
//********************************************************************************************************************
bool DriverPOSTerminal::SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

//********************************************************************************************************************
// 📄 Описание: 
// ПолучитьДополнительныеДействия (GetAdditionalActions)
//
// Метод получает список дополнительных действий, которые будут отображаться в виде пунктов меню 
// в форме настройки оборудования, доступной администратору. 
// Если дополнительные действия не предусмотрены, метод возвращает пустую строку.
//
// 🏷 Имя процедуры: ПолучитьДополнительныеДействия (GetAdditionalActions)
//
// 📤 Выходные параметры:
//   1) Тип: std::wstring
//      Имя: таблицаДействий (TableActions)
//      Направление: [OUT]
//      Описание: XML-строка со списком дополнительных действий. 
//                Формат XML может быть следующим:
//                <TableActions>
//                    <Action>
//                        <ActionID>1</ActionID>
//                        <ActionName>Обновить прошивку</ActionName>
//                        <Description>Позволяет обновить прошивку оборудования до последней версии.</Description>
//                    </Action>
//                    <Action>
//                        <ActionID>2</ActionID>
//                        <ActionName>Сброс настроек</ActionName>
//                        <Description>Выполняет сброс настроек оборудования к заводским параметрам.</Description>
//                    </Action>
//                </TableActions>
//                Если дополнительных действий нет — возвращается пустая строка.
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true — если список действий успешно сформирован и возвращен;  
//       false — при возникновении ошибок (например, сбоя генерации XML).
//
//********************************************************************************************************************
bool DriverPOSTerminal::GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

//********************************************************************************************************************
// 📄 Описание: 
// ВыполнитьДополнительноеДействие (DoAdditionalAction)
//
// Метод выполняет дополнительное действие, указанное по имени.  
// Действия должны быть предварительно определены и доступны в списке дополнительных действий, 
// возвращаемом методом «ПолучитьДополнительныеДействия».
//
// 🏷 Имя процедуры: ВыполнитьДополнительноеДействие (DoAdditionalAction)
//
// 📥 Входные параметры:
//   1) Тип: STRING
//      Имя: имяДействия (ActionName)
//      Направление: [IN]
//      Описание: Строка с именем действия, которое необходимо выполнить.  
//                Имя должно соответствовать одному из действий, определенных в XML-таблице дополнительных действий.
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true — если действие успешно выполнено;  
//       false — если действие не найдено или при его выполнении произошла ошибка.
//
// 📌 Примечание: 
//   Если указано несуществующее имя действия, метод должен вернуть false и зафиксировать ошибку в журнале.
//********************************************************************************************************************
bool DriverPOSTerminal::DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

//********************************************************************************************************************
// 📄 Описание: 
// ПолучитьШаблонЛокализации (GetLocalizationPattern)
//
// Метод возвращает шаблон локализации в формате XML.  
// Шаблон содержит идентификаторы тестовых ресурсов, предназначенные для последующего заполнения значениями 
// в зависимости от выбранной локали.
//
// 🏷 Имя процедуры: ПолучитьШаблонЛокализации (GetLocalizationPattern)
//
// 📥 Входные параметры: отсутствуют
//
// 📤 Выходные параметры:
//   1) Тип: std::wstring&
//      Имя: ШаблонЛокализации (LocalizationPattern)
//      Направление: [OUT]
//      Описание: Строка в формате XML, содержащая структуру с идентификаторами локализуемых ресурсов.  
//                Пример структуры:
//                <Localization>
//                    <Resource id="IDS_DRIVER_NAME"/>
//                    <Resource id="IDS_DRIVER_DESCRIPTION"/>
//                    <Resource id="IDS_DRIVER_VERSION"/>
//                </Localization>
//
// 🔄 Возвращаемое значение:
//   Тип: bool
//   Описание: 
//       true  — если шаблон успешно сформирован и возвращен;  
//       false — если при формировании шаблона возникла ошибка.
//
// 📌 Примечание: 
//   При возникновении ошибки метод должен зафиксировать информацию в журнале ошибок.
//********************************************************************************************************************
bool DriverPOSTerminal::GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

//********************************************************************************************************************
// 📄 Описание: 
// УстановитьЛокализацию (SetLocalization)
//
// Метод устанавливает код языка интерфейса и шаблон локализации для текущего пользователя драйвера.  
// Используемый шаблон локализации должен быть передан в виде XML-строки, содержащей ключи и значения 
// локализуемых ресурсов.
//
// 🏷 Имя процедуры: УстановитьЛокализацию (SetLocalization)
//
// 📥 Входные параметры:
//   1) Тип: STRING  
//      Имя: КодЯзыка (LanguageCode)  
//      Направление: [IN]  
//      Описание: Код языка интерфейса в формате ISO 639-1 (например, "en" — английский, "ru" — русский).
//
//   2) Тип: STRING  
//      Имя: ШаблонЛокализации (LocalizationPattern)  
//      Направление: [IN]  
//      Описание: XML-строка, содержащая локализационные ресурсы, соответствующие выбранному коду языка.  
//                Пример XML-структуры:  
//                <Localization>
//                    <Resource id="IDS_DRIVER_NAME">Драйвер POS-терминала</Resource>
//                    <Resource id="IDS_DRIVER_DESCRIPTION">Описание драйвера</Resource>
//                </Localization>
//
// 🔄 Возвращаемое значение:
//   Тип: bool  
//   Описание: 
//       true  — если язык и шаблон локализации успешно установлены;  
//       false — если при установке возникла ошибка (например, некорректный формат XML).
//
// 📌 Примечание: 
//   - Метод должен валидировать формат XML-шаблона перед установкой.  
//   - При ошибке должны быть зафиксированы соответствующие сообщения в системе логирования.
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
