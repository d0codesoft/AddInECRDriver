#include "ComponentBaseTester.h"
#include "ComponentBase.h"


std::unordered_map<std::wstring, std::wstring> deviceTypesEn = {
        {L"DriverBase", L"������� ������ �������� (�� ���������)"},
        {L"BarcodeScanner", L"������ ���������"},
        {L"CardReader", L"����������� ��������� ����"},
        {L"KKT", L"����������-�������� ������� *"},
        {L"ReceiptPrinter", L"������� �����"},
        {L"CustomerDisplay", L"������� ����������"},
        {L"DataCollectionTerminal", L"�������� ����� ������"},
        {L"POSTerminal", L"������������� �������� *"},
        {L"WeighingScales", L"����������� ����"},
        {L"LabelPrintingScale", L"���� � ������� ��������"},
        {L"LabelPrinter", L"������� ��������"},
        {L"RFIDReader", L"����������� RFID"}
};

std::unordered_map<std::wstring, std::wstring> deviceTypesRu = {
        {L"DriverBase", L"������� ������ �������� (�� ���������)"},
        {L"���������������", L"������ ���������"},
        {L"������������������������", L"����������� ��������� ����"},
        {L"���", L"����������-�������� ������� *"},
        {L"������������", L"������� �����"},
        {L"�����������������", L"������� ����������"},
        {L"�������������������", L"�������� ����� ������"},
        {L"���������������������", L"������������� �������� *"},
        {L"���������������", L"����������� ����"},
        {L"��������������������", L"���� � ������� ��������"},
        {L"���������������", L"������� ��������"},
        {L"�����������RFID", L"����������� RFID"}
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
		std::wstring wstr = str_utils::to_wstring(var.pwstrVal);
        return wstr;
    }
    case VTYPE_PSTR:
#ifdef _WIN32
        return str_utils::to_wstring(std::string(var.pstrVal), "windows-1251");
#else
		return str_utils::to_wstring(std::string(var.pstrVal));
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
		return TV_UI4(&var);
	case VTYPE_UI2:
		return TV_UI2(&var);
	case VTYPE_UI1:
		return TV_UI1(&var);
	case VTYPE_UINT:
		return TV_UINT(&var);
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
