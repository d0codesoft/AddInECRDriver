#pragma once

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <types.h>
#include <map>
#include <string>
#include <variant>
#include <functional>
#include <optional>
#include <span>

//using GetParamFunc = bool (*)(tVariant* pvarParamDefValue);
//typedef bool (*CallAsProcFunc)(tVariant* paParams, const long lSizeArray);
//typedef bool (*CallAsFuncFunc)(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);

using CallAsProc1C = std::function<bool(tVariant*, const long)>;
using CallAsFunc1C = std::function<bool(tVariant*, tVariant*, const long)>;
using CallParamFunc = std::function<bool(tVariant*)>;

using ParamDefault = std::variant<int, std::u16string>;

// Define structure for the property
struct PropName {
    uint32_t propId;
    std::u16string name_en;
    std::u16string name_ru;
    std::u16string descr;
	bool isReadable;
	bool isWritable;
    CallParamFunc getPropValFunc;
    CallParamFunc setPropValFunc;
};

// Define structure for the method procedure
struct MethodName {
	uint32_t methodId;
    std::u16string name_en;
    std::u16string name_ru;
    std::u16string descr;
    bool hasRetVal;
    uint32_t paramCount;
	std::variant<CallAsProc1C, CallAsFunc1C> ptr_method;
    std::map<uint32_t, ParamDefault> default_value_param;
};

struct DriverDescription {
    std::wstring Name;
    std::wstring Description;
    std::wstring EquipmentType;
    bool IntegrationComponent;
    bool MainDriverInstalled;
    std::wstring DriverVersion;
    std::wstring IntegrationComponentVersion;
    bool IsEmulator;
    bool LocalizationSupported;
    bool AutoSetup;
    std::wstring DownloadURL;
    std::wstring EnvironmentInformation;
    bool LogIsEnabled;
    std::wstring LogPath;
    std::wstring ExtensionName;
};

// 🏭 Types of equipment
enum class EquipmentTypeInfo {
    BarcodeScanner,
    CardReader,
    KKT,
    ReceiptPrinter,
    CustomerDisplay,
    DataCollectionTerminal,
    POSTerminal,
    WeighingScales,
    LabelPrintingScale,
    LabelPrinter,
    RFIDReader,
};

// 🌐 Localized lang code
enum class LanguageCode {
	EN,
	RU,
	Unknown
};

// 🌐 Localized types of equipment
struct EquipmentType {
    std::u16string english;
    std::u16string russian;
    EquipmentTypeInfo type;
};

// 🔗 Static list of equipment types
static const std::vector<EquipmentType> EquipmentTypes = {
    {u"BarcodeScanner",            u"СканерШтрихкода",          EquipmentTypeInfo::BarcodeScanner},
    {u"CardReader",                u"СчитывательМагнитныхКарт", EquipmentTypeInfo::CardReader},
    {u"KKT",                       u"ККТ",                      EquipmentTypeInfo::KKT},
    {u"ReceiptPrinter",            u"ПринтерЧеков",             EquipmentTypeInfo::ReceiptPrinter},
    {u"CustomerDisplay",           u"ДисплейПокупателя",        EquipmentTypeInfo::CustomerDisplay},
    {u"DataCollectionTerminal",    u"ТерминалСбораДанных",      EquipmentTypeInfo::DataCollectionTerminal},
    {u"POSTerminal",               u"ЭквайринговыйТерминал",    EquipmentTypeInfo::POSTerminal},
    {u"WeighingScales",            u"ЭлектронныеВесы",          EquipmentTypeInfo::WeighingScales},
    {u"LabelPrintingScale",        u"ВесыСПечатьюЭтикеток",     EquipmentTypeInfo::LabelPrintingScale},
    {u"LabelPrinter",              u"ПринтерЭтикеток",          EquipmentTypeInfo::LabelPrinter},
    {u"RFIDReader",                u"СчитывательRFID",          EquipmentTypeInfo::RFIDReader},
};

bool isValidEquipmentType(const std::u16string& input);
LanguageCode detectLanguage(const std::u16string& input);
std::u16string findEquivalent(const std::u16string& input);
std::optional<EquipmentTypeInfo> getEquipmentTypeInfo(const std::u16string& input);

enum class TypeParameter {
	String,
	Number,
	Bool
};

enum class DriverOption {
    ConnectionType,
    Address,
    Port,
    Speed,
    LogLevel,
    MerchantId,
    Facepay,
    LogFullPath,
    other
};

const std::unordered_map<DriverOption, std::wstring> OptionDriverNames = {
    { DriverOption::ConnectionType, L"ConnectionType" },
    { DriverOption::Address, L"Address" },
    { DriverOption::Port, L"Port" },
    { DriverOption::Speed, L"Speed" },
    { DriverOption::LogLevel, L"LogLevel" },
    { DriverOption::MerchantId, L"MerchantId" },
    { DriverOption::Facepay, L"Facepay" },
    { DriverOption::LogFullPath, L"LogFullPath" }
};

struct DriverParameter {
    std::wstring name;
    std::variant<std::wstring,long,bool> value;
	TypeParameter type;
};

struct ActionDriver {
	std::wstring name_en;
	std::wstring name_ru;
	std::wstring caption_en;
	std::wstring caption_ru;
	CallAsFunc1C action;
};

enum class LogLevel {
    Error = 0,
    Debug
};

const std::unordered_map<LogLevel, std::wstring> LogLevelNames = {
		{ LogLevel::Error, L"Error" },
		{ LogLevel::Debug, L"Debug" }
};

std::wstring getLogLevelIndex(LogLevel level);
std::wstring getLogLevelName(LogLevel level);

template <typename T>
bool toFacepayValue(const T& value) {
	if constexpr (std::is_same<T, std::wstring>::value) {
		return value == L"true" || value == L"1";
	}
	else if constexpr (std::is_same<T, long>::value) {
		return value == 1;
	}
	else if constexpr (std::is_same<T, bool>::value) {
		return value;
	}
    return false;
}

template <typename T>
int toLogLevel(const T& value) {
	if constexpr (std::is_same<T, std::wstring>::value) {
		return value == L"1" ? LogLevel::Debug : LogLevel::Error;
	}
	else if constexpr  (std::is_same<T, long>::value) {
		return value == 1 ? LogLevel::Debug : LogLevel::Error;
	}
	else if constexpr (std::is_same<T, bool>::value) {
		return value ? LogLevel::Debug : LogLevel::Error;
	}
	return LogLevel::Error;
}

std::vector<DriverParameter> ParseParameters(const std::wstring& xmlPath);

bool ParseParametersFromXML(std::vector<DriverParameter>& params, const std::wstring& xmlSource);

template <typename T>
std::optional<T> findParameterValue(const std::vector<DriverParameter>& params, const std::wstring& paramName) {
    for (const auto& param : params) {
        if (param.name == paramName) {
            if (auto value = std::get_if<T>(&param.value)) {
                return *value;
            }
        }
    }
    return std::nullopt;
}

template <typename T>
std::optional<T> findParameterValue(const std::vector<DriverParameter>& params, DriverOption paramType) {
    // Retrieve the parameter name from OptionDriverNames
    auto it = OptionDriverNames.find(paramType);
    if (it == OptionDriverNames.end()) {
        return std::nullopt; // Parameter type not found
    }

    const std::wstring& paramName = it->second;

    // Use the existing findParameterValue function to find the parameter by name
    return findParameterValue<T>(params, paramName);
}

template <typename T>
typename std::enable_if<
    std::is_same<T, std::wstring>::value ||
    std::is_same<T, long>::value ||
    std::is_same<T, bool>::value, bool>::type
    setParameterValue(std::vector<DriverParameter>& params, const std::wstring& paramName, const T& value) {
    for (auto& param : params) {
        if (param.name == paramName) {
			if (param.name == OptionDriverNames.at(DriverOption::ConnectionType)) {
				param.value = toConnectionTypeValue<T>(value);
                param.type = TypeParameter::String;
			}
            else if (param.name == OptionDriverNames.at(DriverOption::Facepay)) {
				param.value = toFacepayValue<T>(value);
				param.type = TypeParameter::Bool;
            }
			else if constexpr (std::is_same<T, std::wstring>::value) {
				param.value = value;
				param.type = TypeParameter::String;
			}
			else if constexpr (std::is_same<T, long>::value) {
				param.value = value;
				param.type = TypeParameter::Number;
			}
			else

            if constexpr (std::is_same<T, std::wstring>::value) {
                param.value = value;
                param.type = TypeParameter::String;
            }
            else if constexpr (std::is_same<T, long>::value) {
                param.value = value;
                param.type = TypeParameter::Number;
            }
            else if constexpr (std::is_same<T, bool>::value) {
                param.value = value;
                param.type = TypeParameter::Bool;
            }
            return true;
        }
    }
    // Determine the type of the parameter
    TypeParameter type;
    if constexpr (std::is_same<T, std::wstring>::value) {
        type = TypeParameter::String;
    }
    else if constexpr (std::is_same<T, long>::value) {
        type = TypeParameter::Number;
    }
    else if constexpr (std::is_same<T, bool>::value) {
        type = TypeParameter::Bool;
    }

    // If the parameter is not found, create a new one
    params.push_back({ paramName, value, type });
    return true;
}

enum class AddinErrorCode : int {
	None = 1000,
	Ordinary = 1001,
	Attention = 1002,
	Important = 1003,
	VeryImportant = 1004,
	Info = 1005,
	Fail = 1006,
	MsgBoxAttention = 1007,
	MsgBoxInfo = 1008,
	MsgBoxFail = 1009
};

/**
 * @brief Структура, описывающая параметры терминала.
 */
struct POSTerminalConfig {
    /**
     * @brief Идентификатор терминала или другого технического средства,
     * предназначенного для совершения операций с использованием платежных карт.
     *
     * @note Не является обязательным параметром.
     */
    std::wstring TerminalID;

    /**
     * @brief Будет ли терминал самостоятельно печатать квитанции на своем принтере для операций.
     */
    bool PrintSlipOnTerminal;

    /**
     * @brief Терминал возвращает короткие слип-чеки, которые будут выводиться в теле фискального чека.
     */
    bool ShortSlip;

    /**
     * @brief Терминал поддерживает функцию выдачи наличных денежных средств.
     */
    bool CashWithdrawal;

    /**
     * @brief Терминал поддерживает оплату электронными сертификатами ФЭС НСПК.
     */
    bool ElectronicCertificates;

    /**
     * @brief Терминал поддерживает частичную отмену.
     */
    bool PartialCancellation;

    /**
     * @brief Терминал поддерживает Consumer-Presented QR-операции на стороне эквайреров.
     */
    bool ConsumerPresentedQR;

    /**
     * @brief Терминал поддерживает получение списка операций по картам.
     */
    bool ListCardTransactions;

    /**
     * @brief Терминал поддерживает операцию возврата ЭС без карты по BasketID оригинальной операции оплаты.
     *
     * @note Не является обязательным параметром.
     */
    bool ReturnElectronicCertificateByBasketID = false;

    /**
     * @brief Терминал поддерживает операцию покупки с зачислением на карту физического лица.
     *
     * @note Не является обязательным параметром.
     */
    bool PurchaseWithEnrollment = false;
};

enum class POSTerminalProtocol {
	JSON,
	BaseECR
};

// Перечисление для типов операций
enum class POSTerminalOperationType {
	Pay = 0,
	ReturnPayment,
	CancelPayment,
	Authorisation,
	AuthConfirmation,
	CancelAuthorisation,
	PayWithCashWithdrawal,
	PayElectronicCertificate,
	ReturnElectronicCertificate,
    NoSet
};

// Массив с представлением типов операций
static const std::unordered_map<POSTerminalOperationType, std::wstring> OperationTypeNames = {
	{ POSTerminalOperationType::Pay, L"Pay" },
	{ POSTerminalOperationType::ReturnPayment, L"ReturnPayment" },
	{ POSTerminalOperationType::CancelPayment, L"CancelPayment" },
	{ POSTerminalOperationType::Authorisation, L"Authorisation" },
	{ POSTerminalOperationType::AuthConfirmation, L"AuthConfirmation" },
	{ POSTerminalOperationType::CancelAuthorisation, L"CancelAuthorisation" },
	{ POSTerminalOperationType::PayWithCashWithdrawal, L"PayWithCashWithdrawal" },
	{ POSTerminalOperationType::PayElectronicCertificate, L"PayElectronicCertificate" },
	{ POSTerminalOperationType::ReturnElectronicCertificate, L"ReturnElectronicCertificate" }
};

// Enum for Indicator Statuses
enum class POSTerminalIndicatorStatus {
	NotSet = 0,          // Статусы не установлены
	Success = 1,         // Операция выполнена успешно
	Failure = 2          // Операция не выполнена
};

// Пример структуры локалей (можно хранить отдельно или грузить из ресурсов)
static const std::unordered_map<std::wstring, std::unordered_map<POSTerminalIndicatorStatus, std::wstring>> IndicatorStatusLocales = {
	{
		L"ru", {
			{ POSTerminalIndicatorStatus::NotSet, L"Статусы не установлены" },
			{ POSTerminalIndicatorStatus::Success, L"Операция выполнена успешно" },
			{ POSTerminalIndicatorStatus::Failure, L"Операция не выполнена" }
		}
	},
	{
		L"ua", {
			{ POSTerminalIndicatorStatus::NotSet, L"Статуси не встановлені" },
			{ POSTerminalIndicatorStatus::Success, L"Операцію виконано успішно" },
			{ POSTerminalIndicatorStatus::Failure, L"Операцію не виконано" }
		}
	},
	{
		L"en", {
			{ POSTerminalIndicatorStatus::NotSet, L"Status not set" },
			{ POSTerminalIndicatorStatus::Success, L"Operation successful" },
			{ POSTerminalIndicatorStatus::Failure, L"Operation failed" }
		}
	}
};

struct POSTerminalOperationParameters {
    POSTerminalOperationType OperationType = POSTerminalOperationType::NoSet;   // Тип операции
    std::optional<long> MerchantNumber;       // Номер мерчанта
    std::optional<long> SubMerchant;		  // Номер субмерчанта (если он есть)
	std::wstring ConsumerPresentedQR;         // QR-код, представленный пользователем
    std::optional<long> UseBiometrics;                  // Использование биометрии (0 или 1)
    std::optional<double> Amount;                       // Сумма операции
    std::optional<double> AmountOriginalTransaction;    // Сумма оригинальной транзакции
    std::optional<double> AmountCash;                   // Сумма наличных
	std::optional<double> Discount; 			        // Сумма скидки
	std::wstring BasketID;                              // Идентификатор корзины
    std::optional<double> ElectronicCertificateAmount;  // Сумма электронного сертификата
    std::optional<double> OwnFundsAmount;               // Сумма собственных средств
    POSTerminalIndicatorStatus OperationStatus = POSTerminalIndicatorStatus::NotSet;   // Статус операции
    std::optional<long> AuthorizationType;    // Тип авторизации
	std::wstring CardNumber;                  // Номер карты
	std::wstring ReceiptNumber;               // Номер чека
	std::wstring RRNCode;                     // Код RRN
	std::wstring AuthorizationCode;           // Код авторизации
	std::wstring Slip;                        // Текст квитанции

	bool isFacepay() const {
		return UseBiometrics.has_value() && UseBiometrics.value() == 1;
	}

	void FacePay(bool useFacePay) {
		UseBiometrics = useFacePay ? 1 : 0;
	}
};

bool isValidPOSTerminalOperationParameters(const POSTerminalOperationParameters& op, const POSTerminalOperationType opType);
bool isValidPOSTerminalOperationParameters(const POSTerminalOperationParameters& op);
bool readPOSTerminalOperationParametersFromXml(const std::wstring& xmlContent, POSTerminalOperationParameters& outParams);
bool writePOSTerminalOperationParametersToXml(const POSTerminalOperationParameters& params, std::wstring& outXml);

std::u16string toXml(const DriverDescription& driver);
std::u16string toXmlApplication(const DriverDescription& driver);
std::u16string toXMLTerminalConfig(const POSTerminalConfig& config);
std::u16string toXMLActions(std::span<const ActionDriver> actions, const LanguageCode currentLang);
std::wstring generateGUID();
std::wstring portToWstring(const std::optional<uint16_t>& port);
std::wstring doubleToAmountString(double value);
long stringToLong(const std::wstring& str);

#define BOOL_TO_STRING(b) ((b) ? L"true" : L"false")

#endif // COMMON_TYPES_H
