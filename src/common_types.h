#pragma once

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <types.h>
#include <map>
#include <string>
#include <variant>
#include <functional>

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

#endif // COMMON_TYPES_H