#pragma once

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <types.h>
#include <map>
#include <string>
#include <variant>
#include <functional>
#include <optional>

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
std::u16string detectLanguage(const std::u16string& input);
std::u16string findEquivalent(const std::u16string& input);
std::optional<EquipmentTypeInfo> getEquipmentTypeInfo(const std::u16string& input);

struct DriverParameter {
    std::wstring name;
    std::wstring value;
};

std::vector<DriverParameter> ParseParameters(const std::wstring& xmlPath);
std::optional<std::wstring> findParameterValue(const std::vector<DriverParameter>& params, const std::wstring& paramName);

#endif // COMMON_TYPES_H
