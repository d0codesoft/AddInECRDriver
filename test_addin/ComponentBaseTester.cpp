#include "ComponentBaseTester.h"
#include "ComponentBase.h"


std::unordered_map<std::wstring, std::wstring> deviceTypesEn = {
        {L"DriverBase", L"Типовые методы драйвера (по умолчанию)"},
        {L"BarcodeScanner", L"Сканер штрихкода"},
        {L"CardReader", L"Считыватель магнитных карт"},
        {L"KKT", L"Контрольно-кассовая техника *"},
        {L"ReceiptPrinter", L"Принтер чеков"},
        {L"CustomerDisplay", L"Дисплей покупателя"},
        {L"DataCollectionTerminal", L"Терминал сбора данных"},
        {L"POSTerminal", L"Эквайринговый терминал *"},
        {L"WeighingScales", L"Электронные весы"},
        {L"LabelPrintingScale", L"Весы с печатью этикеток"},
        {L"LabelPrinter", L"Принтер этикеток"},
        {L"RFIDReader", L"Считыватель RFID"}
};

std::unordered_map<std::wstring, std::wstring> deviceTypesRu = {
        {L"DriverBase", L"Типовые методы драйвера (по умолчанию)"},
        {L"СканерШтрихкода", L"Сканер штрихкода"},
        {L"СчитывательМагнитныхКарт", L"Считыватель магнитных карт"},
        {L"ККТ", L"Контрольно-кассовая техника *"},
        {L"ПринтерЧеков", L"Принтер чеков"},
        {L"ДисплейПокупателя", L"Дисплей покупателя"},
        {L"ТерминалСбораДанных", L"Терминал сбора данных"},
        {L"ЭквайринговыйТерминал", L"Эквайринговый терминал *"},
        {L"ЭлектронныеВесы", L"Электронные весы"},
        {L"ВесыСПечатьюЭтикеток", L"Весы с печатью этикеток"},
        {L"ПринтерЭтикеток", L"Принтер этикеток"},
        {L"СчитывательRFID", L"Считыватель RFID"}
};

std::optional<std::wstring> getDeviceTypeDescription(const std::wstring& deviceType) {
    auto it = deviceTypesEn.find(deviceType);
	if (it != deviceTypesEn.end()) {
		return it->second;
	}
    auto itRu = deviceTypesRu.find(deviceType);
    return (itRu != deviceTypesRu.end()) ? std::optional<std::wstring>(itRu->second) : std::nullopt;
}

std::unordered_map<std::wstring, std::wstring>& getDeviceTypesEn() {
	return deviceTypesEn;
}

std::unordered_map<std::wstring, std::wstring>& getDeviceTypesRu() {
    return deviceTypesRu;
}

bool isValueString(const tVariant& var)
{
	switch (TV_VT(&var))
	{
	case VTYPE_PWSTR:
	case VTYPE_PSTR:
		return true;
	}
    return false;
}

std::wstring getStringValue(const tVariant& var)
{
    switch (TV_VT(&var))
    {
    case VTYPE_PWSTR: {
		std::wstring wstr = str_utils_tools::to_wstring(var.pwstrVal);
        return wstr;
    }
    case VTYPE_PSTR:
#ifdef _WIN32
        return str_utils_tools::to_wstring(std::string(var.pstrVal), "windows-1251");
#else
		return str_utils_tools::to_wstring(std::string(var.pstrVal));
#endif
    }
    return std::wstring();
}

long getLongValue(const tVariant& var)
{
	switch (TV_VT(&var))
	{
	case VTYPE_I4:
		return TV_I4(&var);
	case VTYPE_I2:
		return TV_I2(&var);
	case VTYPE_I1:
		return TV_I1(&var);
	case VTYPE_INT:
		return TV_INT(&var);
	case VTYPE_UI4:
		return static_cast<long>(TV_UI4(&var));
	case VTYPE_UI2:
		return TV_UI2(&var);
	case VTYPE_UI1:
		return TV_UI1(&var);
	case VTYPE_UINT:
		return static_cast<long>(TV_UINT(&var));
	}
	return 0;
}

std::optional<bool> getBoolValue(const tVariant& var)
{
	switch (TV_VT(&var))
	{
	case VTYPE_BOOL:
		return TV_BOOL(&var);
	}
    return std::nullopt;
}
