#pragma once
#include <types.h>
#include <map>

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

using GetParamFunc = bool (*)(tVariant* pvarParamDefValue);
using CallAsProcFunc = bool (*)(tVariant* paParams, const long lSizeArray);
using CallAsFuncFunc = bool (*)(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);

// Define structure for the property
struct PropName {
	UINT propId;
    WCHAR_T* name_en;
    WCHAR_T* name_ru;
    WCHAR_T* descr;
	bool isReadable;
	bool isWritable;
    GetParamFunc getParamDefValueFunc;
    GetParamFunc getGetPropValFunc;
    GetParamFunc getSetPropValFunc;
};

// Define structure for the method procedure
struct MethodName {
	UINT methodId;
    WCHAR_T* name_en;
    WCHAR_T* name_ru;
    WCHAR_T* descr;
    bool hasRetVal;
	union {
		CallAsProcFunc callAsProcFunc;
		CallAsFuncFunc callAsFuncFunc;
	};
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
};

#endif // COMMON_TYPES_H