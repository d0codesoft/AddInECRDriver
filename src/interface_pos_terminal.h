#pragma once

#ifndef IDRIVERPOSTERMINAL_H
#define IDRIVERPOSTERMINAL_H

#include "AddInDefBase.h"

class IDriverPosTerminal
{
public:

//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_3_5
    virtual bool TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#endif
//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_3
    virtual bool Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#elif DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_2_4
	virtual bool PayByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#endif

//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#elif DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_2_4
	virtual bool ReturnPaymentByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#endif

	//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

	//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool CancelPaymentByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;


//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

 //#if DRIVER_REQUIREMENTS_VERSION < DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool AuthorisationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

//#if DRIVER_REQUIREMENTS_VERSION < DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool AuthConfirmationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#if DRIVER_REQUIREMENTS_VERSION < DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool CancelAuthorisationByPaymentCard(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_3_5
    virtual bool CashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    virtual bool PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

	//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool PayByPaymentCardWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

//#if DRIVER_REQUIREMENTS_VERSION >= DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
//#if DRIVER_REQUIREMENTS_VERSION < DRIVER_REQUIREMENTS_VERSION_4_2
    virtual bool GetCardParametrs(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    virtual bool PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
	virtual bool PayElectronicCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    virtual bool ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool ReturnElectronicCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    
    virtual bool EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

//#if DRIVER_REQUIREMENTS_VERSION < DRIVER_REQUIREMENTS_VERSION_4_1
    virtual bool PrintSlipOnTerminal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    virtual ~IDriverPosTerminal() = default;
};

#endif // IDRIVERPOSTERMINAL_H