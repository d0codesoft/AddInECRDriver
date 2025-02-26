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
    std::u16string jsonDescription = convertDriverDescriptionToJson(this->m_driverDescription);
	auto result = m_addInBase->setStringValue(paParams, jsonDescription);
    m_addInBase->setBoolValue(pvarRetValue, result);

	return result;
}

bool DriverPOSTerminal::GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    m_addInBase->setBoolValue(pvarRetValue, true);
    m_addInBase->setStringValue(paParams, m_lastError);
    return true;
}

bool DriverPOSTerminal::EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();

	if (lSizeArray != 2 || !paParams)
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Method expects two parameter", -1);
		addErrorDriver(u"Method EquipmentParameters expects two parameter", L"EquipmentParameters: Method expects two parameter");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

	// Get EquipmentType (STRING[IN])
	std::u16string equipmentType;
	if (paParams[1].vt == VTYPE_PWSTR) {
		equipmentType = std::u16string(paParams[1].pwstrVal);
	}
	else {
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid type for EquipmentType", -1);
		addErrorDriver(u"Invalid type for EquipmentType", L"EquipmentParameters: Invalid type for EquipmentType");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

    auto eqimentInfo = getEquipmentTypeInfo(equipmentType);
	if (!eqimentInfo.has_value() || eqimentInfo.value() != EquipmentTypeInfo::POSTerminal)
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"EquipmentParameters", u"Invalid EquipmentType", -1);
		addErrorDriver(u"Invalid EquipmentType", L"EquipmentParameters: Invalid EquipmentType");
		m_addInBase->setBoolValue(pvarRetValue, false);
		return false;
	}

	// Get SettingXML
	auto settingXml = SettingDriverPos::getSettingXML();
	m_addInBase->setStringValue(paParams, settingXml);
    m_addInBase->setBoolValue(pvarRetValue, true);

	return true;
}

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
    std::u16string equipmentType;
    if (paParams[1].vt == VTYPE_PWSTR) {
        equipmentType = std::u16string(paParams[1].pwstrVal);
    }
    else {
        m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Invalid type for EquipmentType", -1);
        addErrorDriver(u"Invalid type for EquipmentType", L"ConnectEquipment: Invalid type for EquipmentType");
        m_addInBase->setBoolValue(pvarRetValue, false);
        return false;
    }

	if (isValidEquipmentType(equipmentType) == false)
	{
		m_addInBase->addError(ADDIN_E_VERY_IMPORTANT, u"ConnectEquipment", u"Invalid EquipmentType", -1);
        addErrorDriver(u"Invalid EquipmentType", L"ConnectEquipment: Invalid EquipmentType");
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

bool DriverPOSTerminal::EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

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
bool DriverPOSTerminal::TerminalParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::Pay(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::ReturnPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::CancelPayment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::Authorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::AuthConfirmation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::CancelAuthorisation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::PayWithCashWithdrawal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::PurchaseWithEnrollment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::GetCardParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::PayCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::ReturnCertificate(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::EmergencyReversal(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

bool DriverPOSTerminal::GetOperationByCards(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    clearError();
    return false;
}

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
