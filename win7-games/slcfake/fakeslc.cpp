#define WIN32_LEAN_AND_MEAN
#include <windows.h>

HRESULT WINAPI
SLGetWindowsInformationDWORD(LPCWSTR pwszValueName,
                             DWORD  *pdwValue)
{
    *pdwValue = 1;
    return S_OK;
}

BOOL WINAPI
DllMain(HINSTANCE hinstDLL,
        DWORD     fdwReason,
        LPVOID    lpvReserved)
{
    return TRUE;
}
