#pragma once

#ifndef DRIVERPOSTERMINAL_H
#define DRIVERPOSTERMINAL_H

#include "IDriver1CUniBase.h"
#include "IDriverPosTerminal.h"
#include "IAddInBase.h"

class DriverPOSTerminal : public IDriver1CUniBase, public IDriverPosTerminal
{
public:
    DriverPOSTerminal(const IAddInBase* addInBase) : m_addInBase(addInBase) {

    }
    ~DriverPOSTerminal() = default;

	void InitDriver() override;

    const DriverDescription& getDescriptionDriver() override;

    // Implementing methods from IDriver1CUniBase
    bool GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ConnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool DisconnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool SetLocalization(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    const std::map<uint32_t, MethodName>& GetMethods() override;
    const std::map<uint32_t, PropName>& GetProperties() override;

    // Implementing methods from IDriverPosTerminal
    bool TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;

private:

    IAddInBase* m_addInBase = nullptr;

    std::map<uint32_t, MethodName> m_MethodNames = {
        {0, createMethod(0, "GetInterfaceRevision", "ПолучитьРевизиюИнтерфейса",
            "Возвращает поддерживаемую версию требований для данного типа оборудования",
            true, 0, std::bind(&DriverPOSTerminal::GetInterfaceRevision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {1, createMethod(1, "GetDescription", "ПолучитьОписание",
            "Возвращает информацию о драйвере",
            true, 1, std::bind(&DriverPOSTerminal::GetDescription, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {2, createMethod(2, "GetLastError", "ПолучитьОшибку",
            "Возвращает код и описание последней произошедшей ошибки",
            true, 1, std::bind(&DriverPOSTerminal::GetLastError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {3, createMethod(3, "EquipmentParameters", "ПараметрыОборудования",
            "Возвращает список параметров настройки драйвера и их типы, значения по умолчанию и возможные значения",
            true, 2, std::bind(&DriverPOSTerminal::EquipmentParameters, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {4, createMethod(4, "ConnectEquipment", "ПодключитьОборудование",
            "Подключает оборудование с текущими значениями параметров. Возвращает идентификатор подключенного экземпляра устройства",
            true, 3, std::bind(&DriverPOSTerminal::ConnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {5, createMethod(5, "DisconnectEquipment", "ОтключитьОборудование",
            "Отключает оборудование",
            true, 1, std::bind(&DriverPOSTerminal::DisconnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {6, createMethod(6, "EquipmentTest", "ТестированиеОборудования",
            "Выполняет пробное подключение и опрос устройства с текущими значениями параметров, установленными функцией «УстановитьПараметр». При успешном выполнении подключения в описании возвращается информация об устройстве",
            true, 4, std::bind(&DriverPOSTerminal::EquipmentTest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {7, createMethod(7, "EquipmentAutoSetup", "АвтонастройкаОборудования",
            "Выполняет авто-настройку оборудования. Драйвер может показывать технологическое окно, в котором производится автонастройка оборудования. В случае успеха драйвер возвращает параметры подключения оборудования, установленные в результате авто-настройки",
            true, 5, std::bind(&DriverPOSTerminal::EquipmentAutoSetup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {8, createMethod(8, "SetApplicationInformation", "УстановитьИнформациюПриложения",
            "Метод передает в драйвер информацию о приложении, в котором используется данный драйвер",
            true, 1, std::bind(&DriverPOSTerminal::SetApplicationInformation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {9, createMethod(9, "GetAdditionalActions", "ПолучитьДополнительныеДействия",
            "Получает список действий, которые будут отображаться как дополнительные пункты меню в форме настройки оборудования, доступной администратору. Если действий не предусмотрено, возвращает пустую строку",
            true, 1, std::bind(&DriverPOSTerminal::GetAdditionalActions, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {10, createMethod(10, "DoAdditionalAction", "ВыполнитьДополнительноеДействие",
            "Команда на выполнение дополнительного действия с определенным именем",
            true, 1, std::bind(&DriverPOSTerminal::DoAdditionalAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {11, createMethod(11, "GetLocalizationPattern", "ПолучитьШаблонЛокализации",
            "Возвращает шаблон локализации, содержащий идентификаторы тестовых ресурсов для последующего заполнения",
            true, 1, std::bind(&DriverPOSTerminal::GetLocalizationPattern, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {12, createMethod(12, "SetLocalization", "УстановитьЛокализацию",
            "Устанавливает для драйвера код языка для текущего пользователя и шаблон локализации для текущего пользователя",
            true, 1, std::bind(&DriverPOSTerminal::SetLocalization, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) }
    };

    std::map<uint32_t, PropName> m_PropNames = {};
    DriverDescription m_driverDescription;
};

#endif // DRIVERPOSTERMINAL_H