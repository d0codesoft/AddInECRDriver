#pragma once
#include <string>

#ifndef INTERFACEFISCALDRIVER_H
#define INTERFACEFISCALDRIVER_H

// ��������� ��� ���������� ��������
struct OperationParameters {
    std::wstring deviceID;
    std::wstring xmlInput;
    std::wstring xmlOutput;
    std::wstring slipText;
};

// ��������� ��� �������� ����������� ������������
class IFiscalDriver {
public:
    virtual ~IFiscalDriver() = default;
    virtual bool Pay(OperationParameters& params) = 0;
    virtual bool CancelPayment(OperationParameters& params) = 0;
    virtual bool ReturnPayment(OperationParameters& params) = 0;
    virtual bool Authorisation(OperationParameters& params) = 0;
    virtual bool AuthConfirmation(OperationParameters& params) = 0;
    virtual bool CancelAuthorisation(OperationParameters& params) = 0;
    virtual bool Settlement(OperationParameters& params) = 0;
};

#endif // INTERFACEFISCALDRIVER_H
