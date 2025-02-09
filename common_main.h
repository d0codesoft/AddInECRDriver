#pragma once

#ifndef ADDINECRCOMMONSCFUNCTIONS_H
#define ADDINECRCOMMONSCFUNCTIONS_H

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "addin_ecr_common.h"

long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface);
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities);
AttachType GetAttachType();
long DestroyObject(IComponentBase** pIntf);
const WCHAR_T* GetClassNames();


std::wstring getLogDriverFilePath();

#endif // ADDINECRCOMMONSCFUNCTIONS_H