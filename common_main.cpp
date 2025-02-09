#include "common_main.h"
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shlobj.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

static std::u16string s_names(u"ECRCommonSC");
static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if (!*pInterface)
    {
        *pInterface = new CAddInECRCommon;
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

std::wstring getLogDriverFilePath() {
    std::wstring logPath;

#if defined(_WIN32) || defined(_WIN64)
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        logPath = path;
        logPath += L"\\AddInECRCommon\\logs\\";
    }
#elif defined(__linux__) || defined(__APPLE__)
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        homeDir = getpwuid(getuid())->pw_dir;
    }
    logPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(homeDir);
    logPath += L"/.addinecrcommon/logs/";
#endif

    return logPath;
}