#include "pch.h"
#include "manager_hwid.h"
#ifdef _WIN32
#define _WIN32_DCOM
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOKitKeys.h>
#endif
#elif __linux__
#include <fstream>
#include <unistd.h>
#endif
#include <string>
#include <vector>
#include "str_utils.h"
#include "logger.h"

#ifdef __linux__
static std::string read_file_trim(const char* path) {
    std::ifstream f(path);
    if (!f) return {};
    std::string s; std::getline(f, s);
    // trim
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    return s;
}

std::vector<std::string> get_linux_signals() {
    std::vector<std::string> out;

    // DMI product_uuid
    {
        auto s = read_file_trim("/sys/class/dmi/id/product_uuid");
        if (!s.empty()) out.push_back(s);
    }
    // DMI board_serial
    {
        auto s = read_file_trim("/sys/class/dmi/id/board_serial");
        if (!s.empty()) out.push_back(s);
    }
    // Fallback: machine-id
    {
        auto s = read_file_trim("/etc/machine-id");
        if (!s.empty()) out.push_back(s);
    }
    return out;
}
#endif

#ifdef __APPLE__
static std::string get_io_registry_value(CFStringRef key) {
    io_registry_entry_t service = IOServiceGetMatchingService(kIOMainPortDefault,
        IOServiceMatching("IOPlatformExpertDevice"));
    if (!service) return {};

    CFTypeRef cf = IORegistryEntryCreateCFProperty(service, key, kCFAllocatorDefault, 0);
    IOObjectRelease(service);
    if (!cf) return {};

    std::string result;
    if (CFGetTypeID(cf) == CFStringGetTypeID()) {
        char buf[256];
        if (CFStringGetCString((CFStringRef)cf, buf, sizeof(buf), kCFStringEncodingUTF8)) {
            result = buf;
        }
    }
    CFRelease(cf);
    return result;
}

std::vector<std::string> get_macos_signals() {
    std::vector<std::string> out;
    // Hardware UUID
    auto hwUUID = get_io_registry_value(CFSTR(kIOPlatformUUIDKey));
    if (!hwUUID.empty()) out.push_back(hwUUID);

    // Platform serial number — may not be available in sandbox
    auto sn = get_io_registry_value(CFSTR(kIOPlatformSerialNumberKey));
    if (!sn.empty()) out.push_back(sn);

    return out;
}
#endif

#ifdef _WIN32
static std::string read_registry_string(HKEY root, const char* path, const char* name) {
    HKEY hKey{};
    if (RegOpenKeyExA(root, path, 0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS) return {};
    char buf[256]; DWORD sz = sizeof(buf);
    auto rc = RegGetValueA(hKey, nullptr, name, RRF_RT_REG_SZ, nullptr, buf, &sz);
    RegCloseKey(hKey);
    if (rc != ERROR_SUCCESS) return {};
    return std::string(buf);
}

static std::string wmi_single_value(const wchar_t* wql) {
    HRESULT hr;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnum = nullptr;
    std::string result;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return {};

    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);

    pLoc = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) goto cleanup;

    hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);
    if (FAILED(hr)) goto cleanup;

    hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hr)) goto cleanup;

    hr = pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(wql),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum);
    if (FAILED(hr)) goto cleanup;

    {
        IWbemClassObject* pObj = nullptr;
        ULONG ret = 0;
        if (pEnum->Next(5000, 1, &pObj, &ret) == S_OK) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            if (SUCCEEDED(pObj->Get(L"UUID", 0, &vtProp, nullptr, nullptr)) && vtProp.vt == VT_BSTR) {
                _bstr_t b = vtProp.bstrVal;
                result = (const char*)b;
            }
            VariantClear(&vtProp);
            pObj->Release();
        }
    }

cleanup:
    if (pEnum) pEnum->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();
    return result;
}

std::vector<std::string> get_windows_signals() {
    std::vector<std::string> out;

    // SMBIOS UUID
    {
        auto uuid = wmi_single_value(L"SELECT UUID FROM Win32_ComputerSystemProduct");
        if (!uuid.empty()) out.push_back(uuid);
    }
    // MachineGuid
    {
        auto mg = read_registry_string(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid");
        if (!mg.empty()) out.push_back(mg);
    }

    return out;
}
#endif

std::wstring CManagerHwid::GetHWID()
{
	std::vector<std::string> _hwid_signals;

#ifdef _WIN32
	_hwid_signals = get_windows_signals();
#elif __APPLE__
	_hwid_signals = get_macos_signals();
#elif __linux__
	_hwid_signals = get_linux_signals();
#endif
    if (!_hwid_signals.empty()) {
        std::string combined;
        for (const auto& s : _hwid_signals) {
            if (!combined.empty()) combined += "-";
            combined += s;
        }
		LOG_INFO_ADD(L"HWID", L"HWID combined: " + str_utils::to_wstring(combined));
        return str_utils::to_wstring(combined);
	}
    else {
		LOG_INFO_ADD(L"HWID", L"No HWID data found");
    }

	return std::wstring();
}
