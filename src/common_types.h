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

struct DriverParameter {
    std::wstring name;
    std::variant<std::wstring,int,bool> value;
	TypeParameter type;
};

struct ActionDriver {
	std::u16string name_en;
	std::u16string name_ru;
	std::u16string caption_en;
	std::u16string caption_ru;
	CallAsFunc1C action;
};

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
typename std::enable_if<
    std::is_same<T, std::wstring>::value ||
    std::is_same<T, int>::value ||
    std::is_same<T, bool>::value, bool>::type
    setParameterValue(std::vector<DriverParameter>& params, const std::wstring& paramName, const T& value) {
    for (auto& param : params) {
        if (param.name == paramName) {
            param.value = value;
            if constexpr (std::is_same<T, std::wstring>::value) {
                param.type = TypeParameter::String;
            }
            else if constexpr (std::is_same<T, int>::value) {
                param.type = TypeParameter::Number;
            }
            else if constexpr (std::is_same<T, bool>::value) {
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
    else if constexpr (std::is_same<T, int>::value) {
        type = TypeParameter::Number;
    }
    else if constexpr (std::is_same<T, bool>::value) {
        type = TypeParameter::Bool;
    }

    // If the parameter is not found, create a new one
    params.push_back({ paramName, value, type });
    return true;
}


#include <string>

/**
 * @brief Структура, описывающая параметры терминала.
 */
struct TerminalConfig {
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

enum class ProtocolTerminal {
	JSON,
	BaseECR
};

std::u16string toXml(const DriverDescription& driver);
std::u16string toXmlApplication(const DriverDescription& driver);
std::u16string toXMLTerminalConfig(const TerminalConfig& config);
std::u16string toXMLActions(std::span<const ActionDriver> actions, const LanguageCode currentLang);
std::wstring generateGUID();

#define BOOL_TO_STRING(b) ((b) ? L"true" : L"false")

#endif // COMMON_TYPES_H
