#pragma once

#ifndef ADDINECRCOMMONSCFUNCTIONS_H
#define ADDINECRCOMMONSCFUNCTIONS_H

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "addin_driver.h"

// Определяем экспорт для разных ОС
#ifdef _WIN32
	#ifdef LIB_EXPORTS
		#define LIB_API __declspec(dllexport)   // Windows (при сборке DLL)
	#else
		#define LIB_API __declspec(dllimport)   // Windows (при использовании DLL)
	#endif
#else
	#define LIB_API __attribute__((visibility("default"))) // Linux/macOS
#endif


//extern LIB_API long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface);
//extern LIB_API AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities);
//extern LIB_API AttachType GetAttachType();
//extern LIB_API long DestroyObject(IComponentBase** pIntf);
//extern LIB_API const WCHAR_T* GetClassNames();

//std::string getLogDriverFilePath();

#endif // ADDINECRCOMMONSCFUNCTIONS_H