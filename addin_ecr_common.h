#pragma once

#ifndef ADDINECRCOMMONSC_H
#define ADDINECRCOMMONSC_H

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"
#include <map>
#include "common_types.h"

class CAddInECRDriver :
    public IComponentBase
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

    // Methods implement the functionality of the add-in Driver
    bool GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
    bool GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
    bool GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool ConnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool DisconnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }
    bool SetLocalization(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) { return true; }

private:

    void addError(uint32_t wcode, const std::string source,
        const std::string descriptor, long code);

	void initPropNames();
	void initMethodNames();

    // Attributes
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;

	std::map<uint32_t, PropName> m_PropNames;
    std::map<uint32_t, MethodName> m_MethodNames = {
        {0, createMethod(0, "GetInterfaceRevision", "ПолучитьРевизиюИнтерфейса",
            "Возвращает поддерживаемую версию требований для данного типа оборудования",
            true, 0, std::bind(&CAddInECRDriver::GetInterfaceRevision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {1, createMethod(1, "GetDescription", "ПолучитьОписание",
            "Возвращает информацию о драйвере",
            true, 1, std::bind(&CAddInECRDriver::GetDescription, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {2, createMethod(2, "GetLastError", "ПолучитьОшибку",
            "Возвращает код и описание последней произошедшей ошибки",
            true, 1, std::bind(&CAddInECRDriver::GetLastError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {3, createMethod(3, "EquipmentParameters", "ПараметрыОборудования",
            "Возвращает список параметров настройки драйвера и их типы, значения по умолчанию и возможные значения",
            true, 2, std::bind(&CAddInECRDriver::EquipmentParameters, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {4, createMethod(4, "ConnectEquipment", "ПодключитьОборудование",
            "Подключает оборудование с текущими значениями параметров. Возвращает идентификатор подключенного экземпляра устройства",
            true, 3, std::bind(&CAddInECRDriver::ConnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {5, createMethod(5, "DisconnectEquipment", "ОтключитьОборудование",
            "Отключает оборудование",
            true, 1, std::bind(&CAddInECRDriver::DisconnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {6, createMethod(6, "EquipmentTest", "ТестированиеОборудования",
            "Выполняет пробное подключение и опрос устройства с текущими значениями параметров, установленными функцией «УстановитьПараметр». При успешном выполнении подключения в описании возвращается информация об устройстве",
            true, 4, std::bind(&CAddInECRDriver::EquipmentTest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {7, createMethod(7, "EquipmentAutoSetup", "АвтонастройкаОборудования",
            "Выполняет авто-настройку оборудования. Драйвер может показывать технологическое окно, в котором производится автонастройка оборудования. В случае успеха драйвер возвращает параметры подключения оборудования, установленные в результате авто-настройки",
            true, 5, std::bind(&CAddInECRDriver::EquipmentAutoSetup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {8, createMethod(8, "SetApplicationInformation", "УстановитьИнформациюПриложения",
            "Метод передает в драйвер информацию о приложении, в котором используется данный драйвер",
            true, 1, std::bind(&CAddInECRDriver::SetApplicationInformation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {9, createMethod(9, "GetAdditionalActions", "ПолучитьДополнительныеДействия",
            "Получает список действий, которые будут отображаться как дополнительные пункты меню в форме настройки оборудования, доступной администратору. Если действий не предусмотрено, возвращает пустую строку",
            true, 1, std::bind(&CAddInECRDriver::GetAdditionalActions, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {10, createMethod(10, "DoAdditionalAction", "ВыполнитьДополнительноеДействие",
            "Команда на выполнение дополнительного действия с определенным именем",
            true, 1, std::bind(&CAddInECRDriver::DoAdditionalAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {11, createMethod(11, "GetLocalizationPattern", "ПолучитьШаблонЛокализации",
            "Возвращает шаблон локализации, содержащий идентификаторы тестовых ресурсов для последующего заполнения",
            true, 1, std::bind(&CAddInECRDriver::GetLocalizationPattern, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {12, createMethod(12, "SetLocalization", "УстановитьЛокализацию",
            "Устанавливает для драйвера код языка для текущего пользователя и шаблон локализации для текущего пользователя",
            true, 1, std::bind(&CAddInECRDriver::SetLocalization, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) }
    };


    static DriverDescription descriptionDriver;

    std::u16string      m_userLang;
	std::u16string      m_locale;
};

#endif // ADDINECRCOMMONSC_H