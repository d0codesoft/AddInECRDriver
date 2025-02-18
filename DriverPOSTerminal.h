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
        {0, createMethod(0, "GetInterfaceRevision", "�������������������������",
            "���������� �������������� ������ ���������� ��� ������� ���� ������������",
            true, 0, std::bind(&DriverPOSTerminal::GetInterfaceRevision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {1, createMethod(1, "GetDescription", "����������������",
            "���������� ���������� � ��������",
            true, 1, std::bind(&DriverPOSTerminal::GetDescription, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {2, createMethod(2, "GetLastError", "��������������",
            "���������� ��� � �������� ��������� ������������ ������",
            true, 1, std::bind(&DriverPOSTerminal::GetLastError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {3, createMethod(3, "EquipmentParameters", "���������������������",
            "���������� ������ ���������� ��������� �������� � �� ����, �������� �� ��������� � ��������� ��������",
            true, 2, std::bind(&DriverPOSTerminal::EquipmentParameters, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {4, createMethod(4, "ConnectEquipment", "����������������������",
            "���������� ������������ � �������� ���������� ����������. ���������� ������������� ������������� ���������� ����������",
            true, 3, std::bind(&DriverPOSTerminal::ConnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {5, createMethod(5, "DisconnectEquipment", "���������������������",
            "��������� ������������",
            true, 1, std::bind(&DriverPOSTerminal::DisconnectEquipment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {6, createMethod(6, "EquipmentTest", "������������������������",
            "��������� ������� ����������� � ����� ���������� � �������� ���������� ����������, �������������� �������� �������������������. ��� �������� ���������� ����������� � �������� ������������ ���������� �� ����������",
            true, 4, std::bind(&DriverPOSTerminal::EquipmentTest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {7, createMethod(7, "EquipmentAutoSetup", "�������������������������",
            "��������� ����-��������� ������������. ������� ����� ���������� ��������������� ����, � ������� ������������ ������������� ������������. � ������ ������ ������� ���������� ��������� ����������� ������������, ������������� � ���������� ����-���������",
            true, 5, std::bind(&DriverPOSTerminal::EquipmentAutoSetup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {8, createMethod(8, "SetApplicationInformation", "������������������������������",
            "����� �������� � ������� ���������� � ����������, � ������� ������������ ������ �������",
            true, 1, std::bind(&DriverPOSTerminal::SetApplicationInformation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {9, createMethod(9, "GetAdditionalActions", "������������������������������",
            "�������� ������ ��������, ������� ����� ������������ ��� �������������� ������ ���� � ����� ��������� ������������, ��������� ��������������. ���� �������� �� �������������, ���������� ������ ������",
            true, 1, std::bind(&DriverPOSTerminal::GetAdditionalActions, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {10, createMethod(10, "DoAdditionalAction", "�������������������������������",
            "������� �� ���������� ��������������� �������� � ������������ ������",
            true, 1, std::bind(&DriverPOSTerminal::DoAdditionalAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {11, createMethod(11, "GetLocalizationPattern", "�������������������������",
            "���������� ������ �����������, ���������� �������������� �������� �������� ��� ������������ ����������",
            true, 1, std::bind(&DriverPOSTerminal::GetLocalizationPattern, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) },

        {12, createMethod(12, "SetLocalization", "���������������������",
            "������������� ��� �������� ��� ����� ��� �������� ������������ � ������ ����������� ��� �������� ������������",
            true, 1, std::bind(&DriverPOSTerminal::SetLocalization, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)) }
    };

    std::map<uint32_t, PropName> m_PropNames = {};
    DriverDescription m_driverDescription;
};

#endif // DRIVERPOSTERMINAL_H