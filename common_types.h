#pragma once
#include <types.h>
#include <map>
#include <string>
#include <variant>
#include <functional>

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

using GetParamFunc = bool (*)(tVariant* pvarParamDefValue);
typedef bool (*CallAsProcFunc)(tVariant* paParams, const long lSizeArray);
typedef bool (*CallAsFuncFunc)(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);

using CallAsProc1C = std::function<bool(tVariant*, const long)>;
using CallAsFunc1C = std::function<bool(tVariant*, tVariant*, const long)>;

// Define structure for the property
struct PropName {
	UINT propId;
    std::string name_en;
    std::string name_ru;
    std::string descr;
	bool isReadable;
	bool isWritable;
    GetParamFunc getParamDefValueFunc;
    GetParamFunc getGetPropValFunc;
    GetParamFunc getSetPropValFunc;
};

// Define structure for the method procedure
struct MethodName {
	UINT methodId;
    std::string name_en;
    std::string name_ru;
    std::string descr;
    bool hasRetVal;
	UINT paramCount;
	std::variant<CallAsProc1C, CallAsFunc1C> ptr_method;
};

struct DriverDescription {
    std::string Name;
    std::string Description;
    std::string EquipmentType;
    bool IntegrationComponent;
    bool MainDriverInstalled;
    std::string DriverVersion;
    std::string IntegrationComponentVersion;
    bool IsEmulator;
    bool LocalizationSupported;
    bool AutoSetup;
    std::string DownloadURL;
    std::string EnvironmentInformation;
    bool LogIsEnabled;
    std::string LogPath;
};

MethodName createMethod(
    const uint32_t& methodId,
    const std::string& name_en,
    const std::string& name_ru,
    const std::string& descr,
    const bool& hasRetVal,
    const uint32_t& paramCount,
    CallAsFunc1C ptr_method
) {
    return { methodId, name_en, name_ru, descr, hasRetVal, paramCount, ptr_method };
};

#endif // COMMON_TYPES_H