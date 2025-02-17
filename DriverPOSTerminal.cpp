#include "DriverPOSTerminal.h"

#include "IDriverPOSTerminal.h"
#include "string_conversion.h"
#include "resource.h"
#include "SysUtils.h"

void DriverPOSTerminal::InitDriver()
{
	// Implementation here
    m_driverDescription = {
        LoadStringResource(IDS_DRIVER_NAME),
        LoadStringResource(IDS_DRIVER_DESCRIPTION),
        LoadStringResource(IDS_EQUIPMENT_TYPE),
        false, // IntegrationComponent
        false, // MainDriverInstalled
        LoadStringResource(IDS_DRIVER_VERSION),
        LoadStringResource(IDS_DRIVER_INTEGRATION_COMPONENT_VERSION),
        false, // IsEmulator
        true, // LocalizationSupported
        false, // AutoSetup
        LoadStringResource(IDS_DRIVER_DOWNLOAD_URL),
        LoadStringResource(IDS_DRIVER_ENVIRONMENT_INFORMATION),
        true, // LogIsEnabled
        SysUtils::get_full_path(LoadStringResource(IDS_DRIVER_LOG_NAME))
    };
}

const DriverDescription& DriverPOSTerminal::getDescriptionDriver()
{
    return m_driverDescription;
    
}

// Implementing methods from IDriver1CUniBase
bool DriverPOSTerminal::GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Ensure no parameters are expected
    if (lSizeArray != 0)
    {
		this->m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, "GetInterfaceRevision", "Method does not accept parameters", -1);
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
    // Implementation here
    return false;
}

bool DriverPOSTerminal::GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::ConnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::DisconnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::SetLocalization(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

const std::map<uint32_t, MethodName>& DriverPOSTerminal::GetMethods() {
    // Implementation here
    return m_MethodNames;
}

const std::map<uint32_t, PropName>& DriverPOSTerminal::GetProperties() {
    // Implementation here
    return m_PropNames;
}

// Implementing methods from IDriverPosTerminal
bool DriverPOSTerminal::TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}

bool DriverPOSTerminal::Settlement(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    // Implementation here
    return false;
}
