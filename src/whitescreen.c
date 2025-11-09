#include <windows.h>
#include <stdbool.h>

bool g_isFullscreen = false;
int g_screenWidth;
int g_screenHeight;
HWND g_taskbar = NULL;

static void HideTaskbar(bool hide);  // Forward declaration

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char szWindowClass[] = "BLACK_SCREEN_APP";
    const char* szTitle = "Black Screen";

    g_screenWidth = GetSystemMetrics(SM_CXSCREEN);
    g_screenHeight = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        return 0;
    }

    HWND hWnd = CreateWindowEx(
        0,  // No extended style initially
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,  // Start with a reasonable size
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Final cleanup in case WM_DESTROY didn't fire
    if (g_isFullscreen) {
        HideTaskbar(false);
    }

    return 0;
}

static void HideTaskbar(bool hide) {
    if (g_taskbar == NULL) {
        g_taskbar = FindWindow("Shell_TrayWnd", NULL);
    }
    if (g_taskbar != NULL) {
        ShowWindow(g_taskbar, hide ? SW_HIDE : SW_SHOWNA);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_SYSCOMMAND: {
            UINT cmd = wParam & 0xFFF0;
            if (cmd == SC_MAXIMIZE) {
                g_isFullscreen = true;
                LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
                style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
                style |= WS_POPUP;
                SetWindowLongPtr(hWnd, GWL_STYLE, style);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, g_screenWidth, g_screenHeight,
                             SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);

                // Hide taskbar
                HideTaskbar(true);
                return 0;
            } else if (cmd == SC_RESTORE) {
                if (g_isFullscreen) {
                    g_isFullscreen = false;
                    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);
                    style &= ~WS_POPUP;
                    style |= WS_OVERLAPPEDWINDOW;
                    SetWindowLongPtr(hWnd, GWL_STYLE, style);
                    SetWindowPos(hWnd, HWND_NOTOPMOST, 100, 100, 800, 600,
                                 SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOSIZE);

                    // Show taskbar
                    HideTaskbar(false);
                    return 0;
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rect;
            GetClientRect(hWnd, &rect);
            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rect, blackBrush);
            DeleteObject(blackBrush);
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                if (g_isFullscreen) {
                    HideTaskbar(false);
                }
                PostQuitMessage(0);
            }
            break;
        case WM_DESTROY:
            if (g_isFullscreen) {
                HideTaskbar(false);
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}