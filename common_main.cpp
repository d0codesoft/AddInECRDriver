#include "pch.h"
#include "common_main.h"
#include <string>
#if defined(OS_WINDOWS)
#include <windows.h>
#include <shlobj.h>
#elif defined(OS_MACOS) || defined(OS_LINUX)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif
#include "string_conversion.h"

static std::u16string s_names(u"ECRCommonSC");
static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if (!*pInterface)
    {
        *pInterface = new CAddInECRDriver;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
    g_capabilities = capabilities;
    return eAppCapabilitiesLast;
}
//---------------------------------------------------------------------------//
AttachType GetAttachType()
{
    return eCanAttachAny;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
    if (!*pIntf)
        return -1;

    delete* pIntf;
    *pIntf = 0;
    return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    return s_names.c_str();
}

std::string getLogDriverFilePath() {
    std::string logPath;

#if defined(_WIN32) || defined(_WIN64)
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        logPath = wcharToString(path);
        logPath += "\\AddInECRCommon\\logs\\";
    }
#elif defined(__linux__) || defined(__APPLE__)
    const std::string homeDir = getenv("HOME");
    if (!homeDir) {
        homeDir = getpwuid(getuid())->pw_dir;
    }
    logPath = homeDir;
    logPath += "/.addinecrcommon/logs/";
#endif

    return logPath;
}