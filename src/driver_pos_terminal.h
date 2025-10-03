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
#include "�hannel_protocol.h"
#include "connection_types.h"

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
    bool GetCardParametrs(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
    bool Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;
	bool PrintSlipOnTerminal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;

	void AddActionDriver(const std::wstring& name_en, const std::wstring& name_ru, const std::wstring& caption_en, const std::wstring& caption_ru, CallAsFunc1C ptr_method) override;
    std::span<const ActionDriver> getActions() override;

    std::optional<POSTerminalConfig> getTerminalConfig(std::wstring& deviceID) override;

    // Action driver
	bool ActionOpenFileLog(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
	bool ActionOpenConfiguration(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);

protected:

	bool InitConnection(std::wstring& deviceID, std::wstring& error);
	bool testConnection();
    bool testConnection(std::vector<DriverParameter>& paramConnection);
    void addErrorDriver(const std::u16string& lastError, const std::wstring& logError);
	void clearError();
    static std::u16string createUID(const std::wstring& host, uint32_t port);

	std::optional<EquipmentTypeInfo> getEquipmentTypeInfoFromVariant(tVariant* paParam);

    std::optional<std::reference_wrapper<std::unique_ptr<POSTerminalController>>> getDeviceConnection(tVariant* paramDeviceID, std::wstring& deviceId);

    void _handleError(const std::wstring& methodName, const std::wstring& messageError, const bool driverErrorNotify = false, const AddinErrorCode errorCode = AddinErrorCode::VeryImportant);
    inline bool fail(tVariant* pvarRetValue, const std::wstring& context, const std::wstring& errorKey);

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
        // END Implementing methods from IDriver1CUniBase
		
        // START Implementing methods from DriverPosTerminal methods version standart 4.4
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
			true, 2, std::bind(&DriverPOSTerminal::SetLocalization, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(13, u"GetParameters", u"�����������������",
			u"���������� ������ ���������� ��������� �������� � �� ����, �������� �� ��������� � ��������� ��������",
			true, 1, std::bind(&DriverPOSTerminal::GetParameters, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(14, u"SetParameter", u"������������������",
			u"��������� �������� ��������� �� �����",
			true, 2, std::bind(&DriverPOSTerminal::SetParameter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(15, u"Open", u"����������",
			u"���������� ������������ � �������� ���������� ����������, ������������� �������� �������������������. ���������� ������������� ������������� ���������� ����������",
			true, 1, std::bind(&DriverPOSTerminal::Open, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(16, u"Close", u"���������",
			u"��������� ������������",
			true, 1, std::bind(&DriverPOSTerminal::Close, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(17, u"DeviceTest", u"��������������",
			u"��������� ������� ����������� � ����� ���������� � �������� ���������� ����������, �������������� �������� �������������������. ��� �������� ���������� ����������� � �������� ������������ ���������� �� ����������",
			true, 2, std::bind(&DriverPOSTerminal::DeviceTest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(18, u"TerminalParameters", u"������������������",
			u"���������� ��������� ������ ���������",
			true, 2, std::bind(&DriverPOSTerminal::TerminalParameters, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(19, u"Pay", u"��������",
			u"����� ������������ ������",
			true, 4, std::bind(&DriverPOSTerminal::Pay, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(20, u"PayByPaymentCard", u"�����������������������",
			u"����� ������������ ����������� ������ �� �����",
			true, 7, std::bind(&DriverPOSTerminal::PayByPaymentCard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(21, u"ReturnPaymentByPaymentCard", u"�����������������������������",
			u"����� ������������ ������� ������� �� �����",
			true, 7, std::bind(&DriverPOSTerminal::ReturnPaymentByPaymentCard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(22, u"ReturnPayment", u"�������������",
			u"����� ������������ ������� �������",
			true, 4, std::bind(&DriverPOSTerminal::ReturnPayment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(23, u"CancelPayment", u"��������������",
			u"����� ������������ ������. ����������� ��������� ������, �� ����� ������� ������������ ��������, ���� �������� ������������ ������ ��������",
			true, 4, std::bind(&DriverPOSTerminal::CancelPayment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(24, u"CancelPaymentByPaymentCard", u"������������������������������",
			u"����� ������������ ������ ������� �� �����",
			true, 7, std::bind(&DriverPOSTerminal::CancelPaymentByPaymentCard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(25, u"Authorisation", u"��������������",
			u"����� ������������ �������������� � ������������ ����� �� �����",
			true, 4, std::bind(&DriverPOSTerminal::Authorisation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(26, u"AuthorisationByPaymentCard", u"������������������������������",
			u"����� ������������ ���-����������� � ������������ ����� �� ����� �����",
			true, 7, std::bind(&DriverPOSTerminal::AuthorisationByPaymentCard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(27, u"AuthConfirmation", u"�����������������������",
			u"����� ��������� �������������� � ��������� ����� �� �����",
			true, 4, std::bind(&DriverPOSTerminal::AuthConfirmation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(28, u"AuthConfirmationByPaymentCard", u"���������������������������������������",
			u"����� ��������� ���-����������� � ��������� ����� �� ����� �����",
			true, 7, std::bind(&DriverPOSTerminal::AuthConfirmationByPaymentCard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(29, u"CancelAuthorisation", u"����������������������",
			u"����� �������� �������������� � ������������ ����� �� �����",
			true, 4, std::bind(&DriverPOSTerminal::CancelAuthorisation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(30, u"CancelAuthorisationByPaymentCard", u"��������������������������������������",
			u"����� ��������� ���-����������� � ��������� ����� �� ����� �����",
			true, 7, std::bind(&DriverPOSTerminal::CancelAuthorisationByPaymentCard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(31, u"PayWithCashWithdrawal", u"������C���������������",
        	u"����� ������������ �������� ������ � ������� �������� �������� �������. �������� ������ �������� �� �������� ��������������� ��������� � �������������� ������������ ������� �� �����.",
			true, 4, std::bind(&DriverPOSTerminal::PayWithCashWithdrawal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

        IDriver1CUniBase::createMethod(32, u"PayByPaymentCardWithCashWithdrawal", u"������������C���������������",
				u"����� ��������� ���-����������� � ��������� ����� �� ����� �����",
				true, 4, std::bind(&DriverPOSTerminal::PayByPaymentCardWithCashWithdrawal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(33, u"PurchaseWithEnrollment", u"�������������������",
			u"����� ������������ ������� � ����������� �������� ������� �� ����� �������",
			true, 4, std::bind(&DriverPOSTerminal::PurchaseWithEnrollment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(34, u"GetCardParametrs", u"����������������������",
			u"�������� ��������� �����",
			true, 8, std::bind(&DriverPOSTerminal::GetCardParametrs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(35, u"PayCertificate", u"��������������������",
			u"����� ������������ ������ � ����������� ������������ ����������� ��� ����",
			true, 4, std::bind(&DriverPOSTerminal::PayCertificate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(36, u"ReturnCertificate", u"�������������������",
			u"����� ������������ ������� ������ � ����������� ������������ ����������� ��� ����",
			true, 4, std::bind(&DriverPOSTerminal::ReturnCertificate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(37, u"EmergencyReversal", u"�����������������������",
			u"����� �������� ��������� ����������",
			true, 1, std::bind(&DriverPOSTerminal::EmergencyReversal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(38, u"GetOperationByCards", u"������������������������",
			u"��������� ������ ����������� �������� �� ������ �� ����",
			true, 2, std::bind(&DriverPOSTerminal::GetOperationByCards, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(39, u"Settlement", u"����������������",
			u"������������ ������ ������ ���",
			true, 2, std::bind(&DriverPOSTerminal::Settlement, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),

		IDriver1CUniBase::createMethod(40, u"PrintSlipOnTerminal", u"��������������������������",
			u"���������� ����� �� �������� �������������� �������� ��������� ��� ���� �������� ��� ��������",
			true, 0, std::bind(&DriverPOSTerminal::PrintSlipOnTerminal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
    };

    std::vector<PropName> m_PropNames = {};
	std::vector<DriverParameter> m_ParamConnection = {};
	
    ConnectionType m_connectionType = ConnectionType::TCP;
	POSTerminalProtocol m_protocolTerminal = POSTerminalProtocol::JSON;

	std::unordered_map<std::wstring, std::unique_ptr<POSTerminalController>> m_controller = {};
	std::unordered_map<std::wstring, POSTerminalConfig> m_configTerminals = {};

    DriverDescription m_driverDescription;
    std::unique_ptr<LicenseManager> m_licenseManager;
	std::vector<ActionDriver> m_actions = {};

	LanguageCode m_languageCode = LanguageCode::RU;
};

#endif // DRIVERPOSTERMINAL_H