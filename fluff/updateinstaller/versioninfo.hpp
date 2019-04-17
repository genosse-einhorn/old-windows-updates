#pragma once

#include <windows.h>

class VersionInfo
{
    void *data;

    VersionInfo(const VersionInfo &);
    void operator=(const VersionInfo &);

    static char
    hexdigit(int n) {
        if (n < 10)
            return '0' + n;
        else
            return 'a' + (n - 10);
    }

public:
    VersionInfo(const WCHAR *filename): data(NULL)
    {
        DWORD dummy;
        DWORD size = GetFileVersionInfoSizeW((WCHAR*)filename, &dummy);

        data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
        GetFileVersionInfoW((WCHAR*)filename, dummy, size, data);
    }

    ~VersionInfo()
    {
        HeapFree(GetProcessHeap(), 0, data);
    }

    VS_FIXEDFILEINFO
    queryFixedFileInfo() const
    {
        VS_FIXEDFILEINFO retval;
        ZeroMemory(&retval, sizeof(retval));

        void *retp;
        UINT  retp_len;

        if (VerQueryValueW(data, L"\\", &retp, &retp_len)) {
            CopyMemory(&retval, retp, retp_len < sizeof(retval) ? retp_len : sizeof(retval));
        }

        return retval;
    }

    const WCHAR *
    query(const WCHAR *key, WORD lang, WORD codepage) const
    {
        WCHAR block[255] = L"\\StringFileInfo\\";
        block[16] = hexdigit((lang & 0xf000) >> 12);
        block[17] = hexdigit((lang & 0x0f00) >> 8);
        block[18] = hexdigit((lang & 0x00f0) >> 4);
        block[19] = hexdigit((lang & 0x000f));
        block[20] = hexdigit((codepage & 0xf000) >> 12);
        block[21] = hexdigit((codepage & 0x0f00) >> 8);
        block[22] = hexdigit((codepage & 0x00f0) >> 4);
        block[23] = hexdigit((codepage & 0x000f));
        block[24] = '\\';
        for (int i = 0; i < 254-25; ++i) {
            block[25 + i] = key[i];
            if (key[i] == 0)
                break;
        }
        block[254] = 0;

        void *retbuf;
        UINT  retbuf_len;
        if (VerQueryValueW(data, block, &retbuf, &retbuf_len)) {
            return (WCHAR*)retbuf;
        } else {
            return NULL;
        }
    }

    const WCHAR *
    query(const WCHAR *key) const
    {
        struct LANGANDCODEPAGE {
            WORD wLanguage;
            WORD wCodePage;
        } *lpTranslate;
        UINT cbTranslate = 0;

        // Read the list of languages and code pages.
        VerQueryValue(data, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate);

        for (UINT i = 0; i < (cbTranslate / sizeof(*lpTranslate)); ++i) {
            const WCHAR *r = query(key, lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
            if (r)
                return r;
        }

        return NULL;
    }
};
