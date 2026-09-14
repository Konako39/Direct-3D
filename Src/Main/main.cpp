#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <SDKDDKVer.h>

#include "application.h"
#include "direct3d.h"

namespace
{
constexpr wchar_t kWindowClass[] = L"Direct3DProjectWindow";
constexpr wchar_t kWindowTitle[] = L"Direct3D 11 Project";
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
    case WM_SIZE:
        if (w_param != SIZE_MINIMIZED)
        {
            Direct3D_Resize(LOWORD(l_param), HIWORD(l_param));
        }
        return 0;

    case WM_KEYDOWN:
        if (w_param == VK_ESCAPE)
        {
            DestroyWindow(window);
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, w_param, l_param);
}
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show_command)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProcedure;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    window_class.lpszClassName = kWindowClass;

    if (!RegisterClassExW(&window_class))
    {
        return 1;
    }

    RECT window_rect{0, 0, kWindowWidth, kWindowHeight};
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

    const int width = window_rect.right - window_rect.left;
    const int height = window_rect.bottom - window_rect.top;
    const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    HWND window = CreateWindowExW(
        0,
        kWindowClass,
        kWindowTitle,
        WS_OVERLAPPEDWINDOW,
        x,
        y,
        width,
        height,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!window)
    {
        return 1;
    }

    if (!Application_Initialize(window))
    {
        Application_Finalize();
        DestroyWindow(window);
        return 1;
    }

    ShowWindow(window, show_command);
    UpdateWindow(window);

    LARGE_INTEGER frequency{};
    LARGE_INTEGER previous_time{};
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previous_time);

    MSG message{};
    bool running = true;
    while (running)
    {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!running)
        {
            break;
        }

        LARGE_INTEGER current_time{};
        QueryPerformanceCounter(&current_time);
        float delta_time = static_cast<float>(current_time.QuadPart - previous_time.QuadPart)
            / static_cast<float>(frequency.QuadPart);
        previous_time = current_time;

        if (delta_time > 0.1f)
        {
            delta_time = 0.1f;
        }

        Application_Update(delta_time);
        Application_Draw();
    }

    Application_Finalize();
    return 0;
}
