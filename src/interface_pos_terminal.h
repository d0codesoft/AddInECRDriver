#pragma once

#ifndef IDRIVERPOSTERMINAL_H
#define IDRIVERPOSTERMINAL_H

#include "AddInDefBase.h"

class IDriverPosTerminal
{
public:
    virtual bool TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    virtual ~IDriverPosTerminal() = default;
};

#endif // IDRIVERPOSTERMINAL_H