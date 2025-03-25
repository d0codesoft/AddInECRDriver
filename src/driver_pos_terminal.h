#pragma once

#ifndef DRIVERPOSTERMINAL_H
#define DRIVERPOSTERMINAL_H

#include "interface_addin_base.h"
#include "interface_driver_base.h"
#include "interface_pos_terminal.h"
#include "common_types.h"
#include "common_main.h"
#include "interface_connection.h"
#include "setting_driver_pos.h"
#include "license_manager.h"

const std::wstring DRIVER_VERSION = L"1.0.0"; // ���������� ��������� ������ ��������
//using ParameterMap = std::unordered_map<std::wstring, std::variant<std::wstring, int, bool>>;


class DriverPOSTerminal : public IDriver1CUniBase, public IDriverPosTerminal
{
public:
    DriverPOSTerminal(IAddInBase* addInBase);

    ~DriverPOSTerminal();

	void InitDriver() override;

    const DriverDescription& getDescriptionDriver() override;

    // Implementing methods from IDriver1CUniBase
    bool GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    
    //methods version standart 3.2
    bool GetParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool SetParameter(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Open(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Close(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool DeviceTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;

    //methods version standart 4.4
    bool EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ConnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool DisconnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool SetLocalization(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    
    const std::vector<MethodName>& GetMethods() override;
    const std::vector<PropName>& GetProperties() override;

    // Implementing methods from IDriverPosTerminal
    bool TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnPaymentByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool CancelPaymentByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool AuthorisationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool AuthConfirmationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool CancelAuthorisationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayByPaymentCardWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetCardParametrs(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayElectronicCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnElectronicCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
	bool PrintSlipOnTerminal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    const std::u16string getEquipmentId();

	void AddActionDriver(const std::u16string& name_en, const std::u16string& name_ru, const std::u16string& caption_en, const std::u16string& caption_ru, CallAsFunc1C ptr_method) override;
    std::span<const ActionDriver> getActions() override;

protected:

	bool InitConnection(std::wstring& deviceID, std::wstring& error);
	bool testConnection();
    bool testConnection(std::vector<DriverParameter>& paramConnection);
    void addErrorDriver(const std::u16string& lastError, const std::wstring& logError);
	void clearError();
    static std::u16string createUID(const std::wstring& host, uint32_t port);

	std::optional<EquipmentTypeInfo> getEquipmentTypeInfoFromVariant(tVariant* paParam);

private:

    IAddInBase* m_addInBase = nullptr;

	std::u16string m_lastError = u"";

    std::vector<MethodName> m_MethodNames = {
        IDriver1CUniBase::createMethod(0, u"GetInterfaceRevision", u"�������������������������",
            u"���������� �������������� ������ ���������� ��� ������� ���� ������������",
            true, 0, std::bind(&DriverPOSTerminal::GetInterfaceRevision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(1, u"GetDescription", u"����������������",
            u"���������� ���������� � ��������",
            true, 1, std::bind(&DriverPOSTerminal::GetDescription, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(2, u"GetLastError", u"��������������",
            u"���������� ��� � �������� ��������� ������������ ������",
            true, 1, std::bind(&DriverPOSTerminal::GetLastError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(3, u"EquipmentParameters", u"���������������������",
            u"���������� ������ ���������� ��������� �������� � �� ����, �������� �� ��������� � ��������� ��������",
            true, 2, std::bind(&DriverPOSTerminal::EquipmentParameters, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(4, u"ConnectEquipment", u"����������������������",
            u"���������� ������������ � �������� ���������� ����������. ���������� ������������� ������������� ���������� ����������",
            true, 3, std::bind(&DriverPOSTerminal::ConnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(5, u"DisconnectEquipment", u"���������������������",
            u"��������� ������������",
            true, 1, std::bind(&DriverPOSTerminal::DisconnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(6, u"EquipmentTest", u"������������������������",
            u"��������� ������� ����������� � ����� ���������� � �������� ���������� ����������, �������������� �������� �������������������. ��� �������� ���������� ����������� � �������� ������������ ���������� �� ����������",
            true, 4, std::bind(&DriverPOSTerminal::EquipmentTest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(7, u"EquipmentAutoSetup", u"�������������������������",
            u"��������� ����-��������� ������������. ������� ����� ���������� ��������������� ����, � ������� ������������ ������������� ������������. � ������ ������ ������� ���������� ��������� ����������� ������������, ������������� � ���������� ����-���������",
            true, 5, std::bind(&DriverPOSTerminal::EquipmentAutoSetup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(8, u"SetApplicationInformation", u"������������������������������",
            u"����� �������� � ������� ���������� � ����������, � ������� ������������ ������ �������",
            true, 1, std::bind(&DriverPOSTerminal::SetApplicationInformation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(9, u"GetAdditionalActions", u"������������������������������",
            u"�������� ������ ��������, ������� ����� ������������ ��� �������������� ������ ���� � ����� ��������� ������������, ��������� ��������������. ���� �������� �� �������������, ���������� ������ ������",
            true, 1, std::bind(&DriverPOSTerminal::GetAdditionalActions, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(10, u"DoAdditionalAction", u"�������������������������������",
            u"������� �� ���������� ��������������� �������� � ������������ ������",
            true, 1, std::bind(&DriverPOSTerminal::DoAdditionalAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(11, u"GetLocalizationPattern", u"�������������������������",
            u"���������� ������ �����������, ���������� �������������� �������� �������� ��� ������������ ����������",
            true, 1, std::bind(&DriverPOSTerminal::GetLocalizationPattern, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(12, u"SetLocalization", u"���������������������",
            u"������������� ��� �������� ��� ����� ��� �������� ������������ � ������ ����������� ��� �������� ������������",
            true, 1, std::bind(&DriverPOSTerminal::SetLocalization, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) 
    };

    std::vector<PropName> m_PropNames = {};
	std::vector<DriverParameter> m_ParamConnection = {};
    //std::unique_ptr<IConnection> m_connection = nullptr;
	ConnectionType m_connectionType = ConnectionType::TCP;
	//std::u16string m_equipmentId = u"";
	std::unordered_map<std::wstring, std::unique_ptr<IConnection>> m_connections = {};
    //ParameterMap m_parameters = {};

    DriverDescription m_driverDescription;
    std::unique_ptr<LicenseManager> m_licenseManager;
	std::vector<ActionDriver> m_actions = {};
};

#endif // DRIVERPOSTERMINAL_H