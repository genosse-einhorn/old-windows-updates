#define WIN32_LEAN_AND_MEAN

#include <windows.h>

static void
die(const wchar_t *text)
{
    MessageBox(NULL, text, L"Fatal Error", MB_ICONHAND | MB_OK);
    ExitProcess(255);
}

static void
word2hex(WORD w, WCHAR hex[4])
{
    for (int i = 0; i < 4; ++i) {
        WORD c = (w >> ((3-i)*4)) & 0xf;
        if (c >= 10)
            hex[i] = 'A' + (c-10);
        else
            hex[i] = '0' + c;
    }
}

static SIZE_T
wcsCbCat(WCHAR *dst, const WCHAR *src, SIZE_T cbDst)
{
    SIZE_T n = cbDst / sizeof(WCHAR);
    SIZE_T r = 0;

    while (n > 0 && *dst) {
        dst++;
        n--;
        r++;
    }

    while (n > 1 && *src) {
        *dst++ = *src++;
        n--;
        r++;
    }

    while (*src) {
        r++;
        src++;
    }

    *dst = 0;
    r++;

    return r * sizeof(WCHAR);
}

static HANDLE
createTempFile(WCHAR *pathbuf, DWORD cbPathbuf, const WCHAR *prefix, const WCHAR *suffix)
{
    WORD num = (WORD)GetTickCount();
    for (;;) {
        WCHAR numstr[5] = { 0, 0, 0, 0, 0 };
        word2hex(num, numstr);

        pathbuf[0] = 0;
        wcsCbCat(pathbuf, prefix, cbPathbuf);
        wcsCbCat(pathbuf, numstr, cbPathbuf);
        wcsCbCat(pathbuf, suffix, cbPathbuf);

        HANDLE h = CreateFile(pathbuf,
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_NEW,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if ((h != NULL && h != INVALID_HANDLE_VALUE) || GetLastError() != ERROR_FILE_EXISTS)
            return h;

        num += 1;
    }
}

static void
writeStr(HANDLE hFile, const WCHAR *str)
{
    DWORD dummy;
    if (!WriteFile(hFile, str, lstrlenW(str)*sizeof(WCHAR), &dummy, NULL))
        die(L"erorr writing file");
}

int
wmain(int argc, WCHAR *argv[])
{
    if (argc < 2) {
        die(L"Missing argument: file to execute");
    }

    WCHAR prefix[MAX_PATH];
    WCHAR vbsPath[MAX_PATH];
    WCHAR exePath[MAX_PATH];

    GetWindowsDirectory(prefix, sizeof(prefix)/sizeof(prefix[0]));
    wcsCbCat(prefix, L"\\TEMP\\RUON", sizeof(prefix));

    HANDLE hOrigExe = CreateFile(argv[1],
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
    if (hOrigExe == NULL || hOrigExe == INVALID_HANDLE_VALUE) {
        die(L"Could not open exe file");
    }

    HANDLE hVbs = createTempFile(vbsPath, sizeof(vbsPath), prefix, L".VBS");
    HANDLE hExe = createTempFile(exePath, sizeof(exePath), prefix, L".EXE");

    if (!hVbs || !hExe || (hVbs == INVALID_HANDLE_VALUE) || (hExe == INVALID_HANDLE_VALUE)) {
        die(L"temp file creation failed");
    }

    // copy exe
    {
        char buf[2048];
        DWORD numBytes;
        DWORD dummy;

        while (ReadFile(hOrigExe,
                        buf,
                        sizeof(buf),
                        &numBytes,
                        NULL) && numBytes > 0) {
            WriteFile(hExe, buf, numBytes, &dummy, NULL);
        }
    }

    // write vbs
    writeStr(hVbs, L"\xFEFF");
    writeStr(hVbs, L"Set objShell = WScript.CreateObject(\"WScript.Shell\")\r\n");
    writeStr(hVbs, L"Set objFso = WScript.CreateObject(\"Scripting.FileSystemObject\")\r\n");
    writeStr(hVbs, L"objShell.Run \"");
    writeStr(hVbs, exePath);
    for (int i = 2; i < argc; ++i) {
        writeStr(hVbs, L" ");

        bool escapeNeeded = false;
        for (const WCHAR *t = argv[i]; *t; t++) {
            if (*t == ' ' || *t == '"' || *t == '\t' || *t == '\n' || *t == '\r' || *t == '\v') {
                escapeNeeded = true;
                break;
            }
        }

        if (escapeNeeded) {
            writeStr(hVbs, L"\"\"");

            for (const WCHAR *t = argv[i]; *t; t++) {
                int numBackslashes = 0;
                while (*t == '\\') {
                    numBackslashes++;
                    t++;
                }

                if (!*t) {
                    for (int i = 0; i < numBackslashes; ++i)
                        writeStr(hVbs, L"\\\\");
                } else if (*t == '"') {
                    for (int i = 0; i < numBackslashes; ++i)
                        writeStr(hVbs, L"\\\\");
                    writeStr(hVbs, L"\\\"\"");
                } else {
                    for (int i = 0; i < numBackslashes; ++i)
                        writeStr(hVbs, L"\\");

                    WCHAR s[2] = { *t, 0 };
                    writeStr(hVbs, s);
                }
            }

            writeStr(hVbs, L"\"\"");
        } else {
            writeStr(hVbs, argv[i]);
        }
    }
    writeStr(hVbs, L"\", 1, True\r\n");
    writeStr(hVbs, L"objFso.DeleteFile \"");
    writeStr(hVbs, exePath);
    writeStr(hVbs, L"\"\r\n");
    writeStr(hVbs, L"objFso.DeleteFile WScript.ScriptFullName\r\n");
    writeStr(hVbs, L"\r\n");

    CloseHandle(hVbs);
    CloseHandle(hExe);
    CloseHandle(hOrigExe);

    // add runonce registry entry
    HKEY hkRunOnce;
    RegCreateKey(HKEY_LOCAL_MACHINE,
                 L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                 &hkRunOnce);
    RegSetValueEx(hkRunOnce, vbsPath, 0, REG_SZ, (BYTE*)vbsPath, (lstrlenW(vbsPath)+1)*sizeof(WCHAR));
    RegCloseKey(hkRunOnce);

    return 0;
}
