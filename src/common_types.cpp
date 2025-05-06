#include "pch.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "pugixml.hpp"
#include "common_types.h"
#include "string_conversion.h"
#include "logger.h"
#include "str_utils.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <combaseapi.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFUUID.h>
#else
#include <uuid/uuid.h>
#endif
#include <iomanip>

bool isValidEquipmentType(const std::u16string& input) {
    return std::any_of(EquipmentTypes.begin(), EquipmentTypes.end(), [&](const EquipmentType& eq) {
        return eq.english == input || eq.russian == input;
        });
}

// 🌐 Language definition: Russian or English
LanguageCode detectLanguage(const std::u16string& input) {
    static const std::u16string ru_variants[] = { u"ru", u"ru_ru", u"rus_rus" };
    static const std::u16string en_variants[] = { u"en", u"en_en" };

    std::u16string lower_lang = input;
    std::transform(lower_lang.begin(), lower_lang.end(), lower_lang.begin(), ::towlower);

    for (const auto& ru : ru_variants) {
        if (lower_lang == ru) return LanguageCode::RU;
    }
    for (const auto& en : en_variants) {
        if (lower_lang == en) return LanguageCode::EN;
    }

    return LanguageCode::Unknown;
}

// 🏷️ Search for English analogs in Russian
std::u16string findEquivalent(const std::u16string& input) {
    for (const auto& eq : EquipmentTypes) {
        if (eq.english == input) return eq.russian;
        if (eq.russian == input) return eq.english;
    }
    return u"Not Found";
}

std::optional<EquipmentTypeInfo> getEquipmentTypeInfo(const std::u16string& input)
{
    for (const auto& type : EquipmentTypes) {
        if (type.english == input || type.russian == input) {
            return type.type;
        }
    }
    return std::nullopt;
}

std::wstring getLogLevelIndex(LogLevel level)
{
	return std::to_wstring(static_cast<int>(level));
}

std::wstring getLogLevelName(LogLevel level)
{
    auto it = LogLevelNames.find(level);
    if (it != LogLevelNames.end()) {
        return it->second;
    }
    return L"";
}

// 📝 XML file parsing function
std::vector<DriverParameter> ParseParameters(const std::wstring& xmlPath) {
    std::vector<DriverParameter> parameters;
    pugi::xml_document doc;

    // 📂 Загрузка XML файла
    pugi::xml_parse_result result = doc.load_file(xmlPath.c_str(), pugi::parse_default, pugi::encoding_utf8);
    if (!result) {
        LOG_ERROR_ADD(L"CommonTypes", L"Ошибка загрузки XML: " + convertStringToWString(result.description()));
        return parameters;
    }

    // 🔍 Поиск всех узлов Parameter
    for (pugi::xml_node paramNode : doc.child(L"Parameters").children(L"Parameter")) {
        DriverParameter param;
        param.name = paramNode.attribute(L"Name").as_string();
        param.value = paramNode.attribute(L"Value").as_string();

        // ✂️ Удаляем лишние пробелы и переносы строк
        param.name.erase(remove_if(param.name.begin(), param.name.end(), iswspace), param.name.end());
        //param.value.erase(remove_if(param.value.begin(), param.value.end(), iswspace), param.value.end());

        parameters.push_back(param);
    }

    return parameters;
}

bool ParseParametersFromXML(std::vector<DriverParameter>& parameters, const std::wstring& xmlSource)
{
    pugi::xml_document doc;

    // 📂 Загрузка XML файла
    pugi::xml_parse_result result = doc.load_file(xmlSource.c_str(), pugi::parse_default, pugi::encoding_utf8);
    if (!result) {
        LOG_ERROR_ADD(L"CommonTypes", L"Ошибка загрузки XML: " + convertStringToWString(result.description()));
        return false;
    }

    // 🔍 Поиск всех узлов Parameter
	// Iterate over all <Page> elements
	for (pugi::xml_node pageNode : doc.child(L"Settings").children(L"Page")) {
		// Iterate over all <Group> elements within each <Page>
		for (pugi::xml_node groupNode : pageNode.children(L"Group")) {
			// Iterate over all <Parameter> elements within each <Group>
			for (pugi::xml_node paramNode : groupNode.children(L"Parameter")) {
				auto paramName = paramNode.attribute(L"Name").as_string();
                auto paramValue = paramNode.attribute(L"Value").as_string();
                
                auto it = std::find_if(parameters.begin(), parameters.end(), [&](const DriverParameter& param) {
                    return param.name == paramName;
                });

                // Если параметр найден, установить его значение
                if (it != parameters.end()) {
			        if (it->type == TypeParameter::String) {
				        it->value = paramValue;
			        }
			        else if (it->type == TypeParameter::Number) {
				        it->value = std::stoi(paramValue);
			        }
        			else if (it->type == TypeParameter::Bool) {
		        		it->value = paramValue == L"true";
			        }
                }
                else {
                // Если параметр не найден, добавить новый параметр
                    DriverParameter param;
                    param.name = paramName;
                    param.value = paramValue;
        			param.type = TypeParameter::String;
                    parameters.push_back(param);
                }
            }
		}
	}
    return true;
}

std::u16string toXmlApplication(const DriverDescription& driver) {
    pugi::xml_document doc;
	//auto decl = doc.append_child(pugi::node_declaration);
	//decl.append_attribute(L"version") = L"1.0";
	//decl.append_attribute(L"encoding") = L"UTF-8";

    auto root = doc.append_child(L"ApplicationSettings");
    root.append_attribute(L"ApplicationName") = driver.Name.c_str();
    root.append_attribute(L"ApplicationVersion") = driver.DriverVersion.c_str();

    std::wostringstream oss;
    doc.save(oss);

    std::wstring xml_str = oss.str();
    return str_utils::to_u16string(xml_str);
}

std::u16string toXMLTerminalConfig(const POSTerminalConfig& config)
{
	pugi::xml_document doc;
	//auto decl = doc.append_child(pugi::node_declaration);
	//decl.append_attribute(L"version") = L"1.0";
	//decl.append_attribute(L"encoding") = L"UTF-8";

	auto root = doc.append_child(L"TerminalParameters");
	root.append_attribute(L"TerminalID") = config.TerminalID.c_str();
	root.append_attribute(L"PrintSlipOnTerminal") = BOOL_TO_STRING(config.PrintSlipOnTerminal);
	root.append_attribute(L"ShortSlip") = BOOL_TO_STRING(config.ShortSlip);
	root.append_attribute(L"CashWithdrawal") = BOOL_TO_STRING(config.CashWithdrawal);
	root.append_attribute(L"ElectronicCertificates") = BOOL_TO_STRING(config.ElectronicCertificates);
	root.append_attribute(L"PartialCancellation") = BOOL_TO_STRING(config.PartialCancellation);
	root.append_attribute(L"ConsumerPresentedQR") = BOOL_TO_STRING(config.ConsumerPresentedQR);
	root.append_attribute(L"ListCardTransactions") = BOOL_TO_STRING(config.ListCardTransactions);
	root.append_attribute(L"ReturnElectronicCertificateByBasketID") = BOOL_TO_STRING(config.ReturnElectronicCertificateByBasketID);
	root.append_attribute(L"PurchaseWithEnrollment") = BOOL_TO_STRING(config.PurchaseWithEnrollment);

	std::wostringstream oss;
	doc.save(oss);

	std::wstring xml_str = oss.str();
	return str_utils::to_u16string(xml_str);
}

bool isValidPOSTerminalOperationParameters(const POSTerminalOperationParameters& op, const POSTerminalOperationType opType)
{
	switch (opType)
	{
	case POSTerminalOperationType::Pay:
		// Validate parameters for "Pay" operation
		return op.MerchantNumber.has_value() && op.Amount.has_value();

	case POSTerminalOperationType::ReturnPayment:
		// Validate parameters for "ReturnPayment" operation
		return op.Amount.has_value() && op.AmountOriginalTransaction.has_value() && !op.ReceiptNumber.empty() && !op.RRNCode.empty();

	case POSTerminalOperationType::CancelPayment:
		// Validate parameters for "CancelPayment" operation
		return !op.ReceiptNumber.empty();

	case POSTerminalOperationType::Authorisation:
		// Validate parameters for "Authorisation" operation
		return op.Amount > 0 && !op.CardNumber.empty();

	case POSTerminalOperationType::AuthConfirmation:
		// Validate parameters for "AuthConfirmation" operation
		return op.Amount > 0 && !op.AuthorizationCode.empty();

	case POSTerminalOperationType::CancelAuthorisation:
		// Validate parameters for "CancelAuthorisation" operation
		return !op.AuthorizationCode.empty();

	case POSTerminalOperationType::PayWithCashWithdrawal:
		// Validate parameters for "PayWithCashWithdrawal" operation
		return op.Amount > 0 && op.AmountCash > 0 && !op.CardNumber.empty();

	case POSTerminalOperationType::PayElectronicCertificate:
		// Validate parameters for "PayElectronicCertificate" operation
		return op.ElectronicCertificateAmount > 0 && !op.BasketID.empty();

	case POSTerminalOperationType::ReturnElectronicCertificate:
		// Validate parameters for "ReturnElectronicCertificate" operation
		return op.ElectronicCertificateAmount > 0 && !op.BasketID.empty();

	default:
		// Unknown operation type
		break;
	}
    return false;
}

bool isValidPOSTerminalOperationParameters(const POSTerminalOperationParameters& op)
{
	if (op.OperationType == POSTerminalOperationType::NoSet) {
		return false;
	}
    return isValidPOSTerminalOperationParameters(op, op.OperationType);
}

std::u16string toXml(const DriverDescription& driver)
{
    pugi::xml_document doc;
    //auto decl = doc.append_child(pugi::node_declaration);
    //decl.append_attribute(L"version") = L"1.0";
    //decl.append_attribute(L"encoding") = L"UTF-8";

    auto root = doc.append_child(L"DriverDescription");
    root.append_attribute(L"Name") = driver.Name.c_str();
    root.append_attribute(L"Description") = driver.Description.c_str();
    root.append_attribute(L"EquipmentType") = driver.EquipmentType.c_str();
    root.append_attribute(L"IntegrationComponent") = BOOL_TO_STRING(driver.IntegrationComponent);
    root.append_attribute(L"MainDriverInstalled") = BOOL_TO_STRING(driver.MainDriverInstalled);
    root.append_attribute(L"DriverVersion") = driver.DriverVersion.c_str();
    root.append_attribute(L"IntegrationComponentVersion") = driver.IntegrationComponentVersion.c_str();
    root.append_attribute(L"IsEmulator") = BOOL_TO_STRING(driver.IsEmulator);
    root.append_attribute(L"LocalizationSupported") = BOOL_TO_STRING(driver.LocalizationSupported);
    root.append_attribute(L"AutoSetup") = BOOL_TO_STRING(driver.AutoSetup);
    root.append_attribute(L"DownloadURL") = driver.DownloadURL.c_str();
    root.append_attribute(L"EnvironmentInformation") = driver.EnvironmentInformation.c_str();
    root.append_attribute(L"LogIsEnabled") = BOOL_TO_STRING(driver.LogIsEnabled);
    root.append_attribute(L"LogPath") = driver.LogPath.c_str();
    root.append_attribute(L"ExtensionName") = driver.ExtensionName.c_str();

    std::wostringstream oss;
    doc.save(oss);

    std::wstring xml_str = oss.str();
    return str_utils::to_u16string(xml_str);
}

std::u16string toXMLActions(std::span<const ActionDriver> actions, const LanguageCode currentLang) {
    pugi::xml_document doc;
    //auto decl = doc.append_child(pugi::node_declaration);
    //decl.append_attribute(L"version") = "1.0";
    //decl.append_attribute(L"encoding") = "UTF-8";

    auto root = doc.append_child(L"Actions");

    for (const auto& action : actions) {
        auto actionNode = root.append_child(L"Action");
        if (currentLang == LanguageCode::EN) {
            actionNode.append_attribute(L"Name") = action.name_en.c_str();
            actionNode.append_attribute(L"Caption") = action.caption_en.c_str();
        }
        else if (currentLang == LanguageCode::RU) {
            actionNode.append_attribute(L"Name") = action.name_ru.c_str();
            actionNode.append_attribute(L"Caption") = action.caption_ru.c_str();
        }
        else
        {
			actionNode.append_attribute(L"Name") = action.name_en.c_str();
			actionNode.append_attribute(L"Caption") = action.caption_en.c_str();
        }
    }

    std::wostringstream oss;
    doc.save(oss);

    std::wstring xml_str = oss.str();
    return str_utils::to_u16string(xml_str);
}

std::wstring generateGUID() {

    std::wstring guidBuff = {};

#if defined(_WIN32) || defined(_WIN64)
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create GUID");
    }

    wchar_t guidString[39]; // GUID string format is 38 characters plus null terminator
    int len = StringFromGUID2(guid, guidString, 39);
    if (len == 0) {
        throw std::runtime_error("Failed to convert GUID to string");
    }

    guidBuff = std::wstring(guidString);
#elif defined(__APPLE__)
    CFUUIDRef uuid = CFUUIDCreate(NULL);
    CFStringRef uuidStr = CFUUIDCreateString(NULL, uuid);
    CFRelease(uuid);

    const char* cStr = CFStringGetCStringPtr(uuidStr, kCFStringEncodingUTF8);
    std::wstring guidString;
    if (cStr) {
        guidString = std::wstring(cStr, cStr + strlen(cStr));
    }
    else {
        CFIndex length = CFStringGetLength(uuidStr);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
        char* buffer = (char*)malloc(maxSize);
        if (CFStringGetCString(uuidStr, buffer, maxSize, kCFStringEncodingUTF8)) {
            guidString = std::wstring(buffer, buffer + strlen(buffer));
        }
        free(buffer);
    }
    CFRelease(uuidStr);

    guidBuff = guidString;

#else // Linux and other Unix-like systems
    uuid_t uuid;
    uuid_generate(uuid);

    char uuidStr[37]; // UUID string format is 36 characters plus null terminator
    uuid_unparse(uuid, uuidStr);

    guidBuff = guidString(uuidStr, uuidStr + strlen(uuidStr));
#endif

    if (guidBuff.size() >= 2 && guidBuff.front() == L'{' && guidBuff.back() == L'}') {
        return guidBuff.substr(1, guidBuff.size() - 2);
    }
    return guidBuff;
}

std::wstring portToWstring(const std::optional<uint16_t>& port)
{
	if (port.has_value()) {
		std::wstringstream wss;
		wss << port.value();
		return wss.str();
	}
	else {
		return L"";
	}

}

std::wstring doubleToAmountString(double value) {
    std::wostringstream woss;
    woss << std::fixed << std::setprecision(2) << value;
    return woss.str();
}

long stringToLong(const std::wstring& str) {
	try {
		// Use std::stoul to convert wstring to unsigned long
		unsigned long value = std::stol(str);
		// Cast to unsigned int if necessary
		return static_cast<unsigned int>(value);
	}
    catch (const std::invalid_argument& e) {
        // Handle invalid argument exception
        LOG_ERROR_ADD(L"CommonTypes", L"Invalid convert argument: " + str + L" to long: " + str_utils::to_wstring(e.what()));
    }

    return 0;
}

std::optional<long> get_long_attr(const pugi::xml_node& node, const std::wstring& name) {
    if (!node.attribute(name).empty()) {
        return node.attribute(name).as_llong(0);
    }
	return std::nullopt;
}

std::optional<double> get_double_attr(const pugi::xml_node& node, const std::wstring& name) {
    if (!node.attribute(name).empty()) {
        return node.attribute(name).as_double(0.0);
    }
	return std::nullopt;
}

std::wstring get_wstring_attr(const pugi::xml_node& node, const std::wstring& name) {
    if (!node.attribute(name).empty()) {
        return node.attribute(name).as_string();
    }
	return L"";
}

bool readPOSTerminalOperationParametersFromXml(const std::wstring& xmlContent, POSTerminalOperationParameters& outParams) {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xmlContent.c_str());

	if (!result) {
		return false;
	}

	pugi::xml_node root = doc.child(L"OperationParameters");
	if (!root) {
		return false;
	}

	outParams.MerchantNumber = get_long_attr(root, L"MerchantNumber");
	outParams.UseBiometrics = get_long_attr(root, L"UseBiometrics");
	outParams.Amount = get_double_attr(root, L"Amount");
	outParams.AmountOriginalTransaction = get_double_attr(root, L"AmountOriginalTransaction");
	outParams.AmountCash = get_double_attr(root, L"AmountCash");
	outParams.Discount = get_double_attr(root, L"Discount");
	outParams.ElectronicCertificateAmount = get_double_attr(root, L"ElectronicCertificateAmount");
	outParams.OwnFundsAmount = get_double_attr(root, L"OwnFundsAmount");
	outParams.AuthorizationType = get_long_attr(root, L"AuthorizationType");

	// Optional fields
	outParams.ConsumerPresentedQR = get_wstring_attr(root, L"ConsumerPresentedQR");
	outParams.BasketID = get_wstring_attr(root, L"BasketID");
	auto _op = get_long_attr(root, L"OperationStatus");
	if (_op.has_value()) {
		outParams.OperationStatus = static_cast<POSTerminalIndicatorStatus>(_op.value());
	}
	outParams.CardNumber = get_wstring_attr(root, L"CardNumber");
	outParams.ReceiptNumber = get_wstring_attr(root, L"ReceiptNumber");
	outParams.RRNCode = get_wstring_attr(root, L"RRNCode");
	outParams.AuthorizationCode = get_wstring_attr(root, L"AuthorizationCode");

	return true;
}

bool writePOSTerminalOperationParametersToXml(const POSTerminalOperationParameters& params, std::wstring& outXml)
{
	pugi::xml_document doc;

	// Create the root node
	auto root = doc.append_child(L"OperationParameters");

	// Add attributes to the root node
	if (params.MerchantNumber.has_value()) {
		root.append_attribute(L"MerchantNumber") = params.MerchantNumber.value();
	}
	if (params.SubMerchant.has_value()) {
		root.append_attribute(L"SubMerchant") = params.SubMerchant.value();
	}
	if (!params.ConsumerPresentedQR.empty()) {
		root.append_attribute(L"ConsumerPresentedQR") = params.ConsumerPresentedQR.c_str();
	}
	if (params.UseBiometrics.has_value()) {
		root.append_attribute(L"UseBiometrics") = params.UseBiometrics.value();
	}
	if (params.Amount.has_value()) {
		root.append_attribute(L"Amount") = params.Amount.value();
	}
	if (params.OperationType == POSTerminalOperationType::CancelPayment && params.AmountOriginalTransaction.has_value()) {
		root.append_attribute(L"AmountOriginalTransaction") = params.AmountOriginalTransaction.value();
	}
	if (params.OperationType == POSTerminalOperationType::PayWithCashWithdrawal && params.AmountCash.has_value()) {
		root.append_attribute(L"AmountCash") = params.AmountCash.value();
	}
	if (params.Discount.has_value()) {
		root.append_attribute(L"Discount") = params.Discount.value();
	}
	if (params.OperationType == POSTerminalOperationType::PayElectronicCertificate && !params.BasketID.empty()) {
		root.append_attribute(L"BasketID") = params.BasketID.c_str();
	}
	if (params.OperationType == POSTerminalOperationType::PayElectronicCertificate && params.ElectronicCertificateAmount.has_value()) {
		root.append_attribute(L"ElectronicCertificateAmount") = params.ElectronicCertificateAmount.value();
	}
	if (params.OperationType == POSTerminalOperationType::PayElectronicCertificate && params.OwnFundsAmount.has_value()) {
		root.append_attribute(L"OwnFundsAmount") = params.OwnFundsAmount.value();
	}
    if (params.OperationType == POSTerminalOperationType::PayElectronicCertificate) {
        root.append_attribute(L"OperationStatus") = static_cast<int>(params.OperationStatus);
    }
	if (params.AuthorizationType.has_value()) {
		root.append_attribute(L"AuthorizationType") = params.AuthorizationType.value();
	}
	if (!params.CardNumber.empty()) {
		root.append_attribute(L"CardNumber") = params.CardNumber.c_str();
	}
	if (!params.ReceiptNumber.empty()) {
		root.append_attribute(L"ReceiptNumber") = params.ReceiptNumber.c_str();
	}
	if (!params.RRNCode.empty()) {
		root.append_attribute(L"RRNCode") = params.RRNCode.c_str();
	}
	if (!params.AuthorizationCode.empty()) {
		root.append_attribute(L"AuthorizationCode") = params.AuthorizationCode.c_str();
	}

    // Convert the XML document to a string
	std::wostringstream oss;
	doc.save(oss);
	outXml = oss.str();

	return true;
}
