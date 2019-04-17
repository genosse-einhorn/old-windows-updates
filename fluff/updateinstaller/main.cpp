#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#if WINVER < 0x0500
    #define COMPILE_MULTIMON_STUBS
    #include <multimon.h>
#endif

#include "string.hpp"
#include "linereader.hpp"
#include "worker.hpp"

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a , b) ((a) > (b) ? (a) : (b))

static inline void die(const wchar_t *text)
{
    MessageBox(NULL, text, L"Fataler Fehler", MB_ICONHAND | MB_OK);
    ExitProcess(255);
}

struct autoSizeWindowClosure {
    RECT bounding;
    HWND subject;
};

static BOOL CALLBACK autoSizeWindowEnumChildProc(HWND hwnd, LPARAM lParam)
{
    struct autoSizeWindowClosure *closure = (struct autoSizeWindowClosure*)lParam;

    RECT c;
    if (GetWindowRect(hwnd, &c) && MapWindowPoints(HWND_DESKTOP, closure->subject, (LPPOINT)&c, 2)) {
        closure->bounding.left = MIN(closure->bounding.left, c.left);
        closure->bounding.top = MIN(closure->bounding.top, c.top);
        closure->bounding.right = MAX(closure->bounding.right, c.right);
        closure->bounding.bottom = MAX(closure->bounding.bottom, c.bottom);
    }

    return TRUE;
}

static inline void autoSizeWindow(HWND hwnd)
{
    struct autoSizeWindowClosure closure = {
        { 2147483647, 2147483647, 0, 0 },
        hwnd
    };

    EnumChildWindows(hwnd, autoSizeWindowEnumChildProc, (LPARAM)&closure);

    if (closure.bounding.left < closure.bounding.right && closure.bounding.top < closure.bounding.bottom) {
        RECT window = { 0, 0, closure.bounding.right + closure.bounding.left, closure.bounding.bottom + closure.bounding.top };
        AdjustWindowRectEx(&window, GetWindowLong(hwnd, GWL_STYLE), GetMenu(hwnd) != NULL, GetWindowLong(hwnd, GWL_EXSTYLE));
        SetWindowPos(hwnd, hwnd, 0, 0, window.right - window.left, window.bottom - window.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
}

static inline void centerWindow(HWND hwnd)
{
    POINT p = {0, 0};
    HMONITOR mon = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFO i;
    ZeroMemory(&i, sizeof(i));
    i.cbSize = sizeof(i);

    if (GetMonitorInfo(mon, &i)) {
        int mid_x = i.rcWork.left / 2 + i.rcWork.right / 2;
        int mid_y = i.rcWork.top / 2 + i.rcWork.bottom / 2;

        RECT winrect;
        if (GetWindowRect(hwnd, &winrect)) {
            int win_width = winrect.right - winrect.left;
            int win_height = winrect.bottom - winrect.top;

            SetWindowPos(hwnd, NULL, mid_x - win_width/2, mid_y - win_height/2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }
    }
}

static inline HICON getSoftwareIcon(void)
{
    SHFILEINFO shf;
    ZeroMemory(&shf, sizeof(shf));

    if (SHGetFileInfo(L".msi", FILE_ATTRIBUTE_NORMAL, &shf, sizeof(shf), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES) && shf.hIcon) {
        return shf.hIcon;
    }

    return NULL;
}

static inline HICON getSoftwareIconSmall(void)
{
    SHFILEINFO shf;
    ZeroMemory(&shf, sizeof(shf));

    if (SHGetFileInfo(L".msi", FILE_ATTRIBUTE_NORMAL, &shf, sizeof(shf), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES) && shf.hIcon) {
        return shf.hIcon;
    }

    return NULL;
}

class MainWindow
{
    Worker   &worker;
    ATOM      wndclass;
    HWND      hwnd;
    HWND      progress_hwnd;
    HWND      text_hwnd;
    LOGFONTW  font;
    HINSTANCE hinst;

public:
    MainWindow(HINSTANCE hinst_, Worker &w) : worker(w)
    {
        hinst = hinst_;

        WNDCLASS cls;
        ZeroMemory(&cls, sizeof(cls));

        cls.lpfnWndProc   = &MainWindow::WndProc;
        cls.style         = CS_NOCLOSE;
        cls.cbWndExtra    = sizeof(MainWindow*);
        cls.hInstance     = hinst;
        cls.lpszClassName = L"UpdateInstallerWindow";
        cls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        cls.hCursor       = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
        cls.hIcon         = getSoftwareIconSmall();

        wndclass = RegisterClass(&cls);
        if (!wndclass)
            die(L"creating window class");

        hwnd = CreateWindowEx(0,
                              MAKEINTATOM(wndclass),
                              worker.caption().cstr(),
                              WS_OVERLAPPED | WS_CAPTION,
                              0, 0,
                              400, 10,
                              NULL,
                              NULL,
                              hinst,
                              (void*)this);
        if (!hwnd)
            die(L"creating window");

        autoSizeWindow(hwnd);
        centerWindow(hwnd);
    }

    ~MainWindow() {
        DestroyWindow(hwnd);
        UnregisterClass(MAKEINTATOM(wndclass), hinst);
    }

    void show(int nCmdShow = SW_SHOW) {
        ShowWindow(hwnd, nCmdShow);
    }

    void work() {
        for (int i = 0; i < worker.itemcount(); ++i) {
            Worker::Item item = worker.prepareitem(i);
            SendMessage(progress_hwnd, PBM_SETPOS, (WPARAM)(i + 1), (LPARAM)0);
            SendMessage(text_hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)(wstr(L"Installiere: ") + item.text).cstr());

            item.process();
        }
    }

private:
    inline int toPx(int emPercent)
    {
        return MulDiv(emPercent, -font.lfHeight, 100);
    }

    LRESULT CALLBACK instWndProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CREATE) {
            NONCLIENTMETRICS ncm;

            ZeroMemory(&ncm, sizeof(ncm));
            ncm.cbSize = sizeof(ncm);

            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0) && ncm.lfMessageFont.lfHeight < 0) {
                CopyMemory(&font, &ncm.lfMessageFont, sizeof(font));
            } else {
                // fallback to default font, 8pt MS Shell Dlg
                ZeroMemory(&font, sizeof(font));

                HDC tmp_dc = GetDC(NULL);
                font.lfHeight = -MulDiv(8, GetDeviceCaps(tmp_dc, LOGPIXELSY), 72);
                lstrcpyW(font.lfFaceName, L"MS Shell Dlg");
                ReleaseDC(NULL, tmp_dc);
            }

            InitCommonControls();

            int iconw = GetSystemMetrics(SM_CXICON);
            int iconh = GetSystemMetrics(SM_CYICON);

            progress_hwnd = CreateWindowEx(0,
                                           PROGRESS_CLASS,
                                           (WCHAR*) NULL,
                                           WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                                           toPx(103), toPx(103) + iconh + toPx(411),
                                           toPx(3328), toPx(141),
                                           hwnd,
                                           (HMENU) 0,
                                           ((CREATESTRUCTW*)lParam)->hInstance,
                                           NULL);
            if (!progress_hwnd)
                die(L"creating progress bar");
            SendMessage(progress_hwnd, PBM_SETRANGE32, (WPARAM)0, (LPARAM)worker.itemcount());
            SendMessage(progress_hwnd, PBM_SETSTEP, (WPARAM)1, (LPARAM)0);
            SendMessage(progress_hwnd, PBM_SETPOS, (WPARAM)1, (LPARAM)0);

            text_hwnd = CreateWindowEx(0,
                                       L"STATIC",
                                       L"Initialisiere...",
                                       WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
                                       toPx(103), toPx(103) + iconh + toPx(242),
                                       toPx(3328), toPx(122),
                                       hwnd,
                                       (HMENU)0,
                                       ((CREATESTRUCTW*)lParam)->hInstance,
                                       NULL);
            if (!text_hwnd)
                die(L"creating static control");

            HFONT f = CreateFontIndirect(&font);
            SendMessage(text_hwnd, WM_SETFONT, (WPARAM)f, (LPARAM)0);

            HWND softicon_hwnd = CreateWindowEx(0,
                                                L"STATIC",
                                                L"Dummy",
                                                WS_CHILD | WS_VISIBLE | SS_ICON,
                                                toPx(103), toPx(103),
                                                0, 0,
                                                hwnd,
                                                (HMENU)0,
                                                ((CREATESTRUCTW*)lParam)->hInstance,
                                                NULL);
            if (!softicon_hwnd)
                die(L"icon static control");

            SendMessage(softicon_hwnd, STM_SETICON, (WPARAM)getSoftwareIcon(), (LPARAM)0);

            HWND softlbl_hwnd = CreateWindowEx(0,
                                               L"STATIC",
                                               worker.text().cstr(),
                                               WS_CHILD | WS_VISIBLE,
                                               toPx(103) + iconw + toPx(47), toPx(103),
                                               toPx(3328)- (iconw + toPx(47)), toPx(103*3),
                                               hwnd,
                                               (HMENU)0,
                                               ((CREATESTRUCTW*)lParam)->hInstance,
                                               NULL);
            SendMessage(softlbl_hwnd, WM_SETFONT, (WPARAM)f, (LPARAM)0);
        }

        if (message == WM_CLOSE) {
            PostQuitMessage(0);
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                    WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CREATE) {
            SetWindowLong(hWnd, 0, (LONG)(((LPCREATESTRUCT) lParam)->lpCreateParams));
            ((MainWindow*)((CREATESTRUCTW*)lParam)->lpCreateParams)->hwnd = hWnd;
        }

        MainWindow *inst = (MainWindow*)(void*)GetWindowLong(hWnd, 0);
        if (inst) {
            return inst->instWndProc(message, wParam, lParam);
        } else {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
};

extern "C"
int CALLBACK
wWinMain(HINSTANCE inst, HINSTANCE, LPWSTR, int nCmdShow)
{
    WCHAR path[MAX_PATH+1];
    ZeroMemory(path, sizeof(path));

    GetModuleFileName((HMODULE)inst, path, MAX_PATH);

    // replace .exe with .lst
    int pathlen = lstrlenW(path);
    path[pathlen-3] = 'l';
    path[pathlen-2] = 's';
    path[pathlen-1] = 't';

    Worker w;

    {
        LineReader r(path);
        if (!r.ok()) {
            die(L"Konnte Steuerdatei nicht \x00F6"L"ffnen.");
        }
        w.load(r);
    }

    // change to containing directory
    for (int i = pathlen-1; i > 0; --i) {
        if (path[i] == '\\') {
            path[i] = 0;
            SetCurrentDirectory(path);
            break;
        }
    }

    MainWindow win(inst, w);
    win.show(nCmdShow);
    win.work();

    return 0;
}
