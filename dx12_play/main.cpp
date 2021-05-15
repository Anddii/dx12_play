#include <stdio.h>
#include <windows.h>

#include "d3d12_motor.h"
#include "game.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

namespace
{
    std::shared_ptr<D3D12Motor> motor;
    std::shared_ptr<Game> game;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	// Register window class
	const wchar_t CLASS_NAME[] = L"Sample Window";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"View Meshlets",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    motor = std::shared_ptr<D3D12Motor>(new D3D12Motor());
    motor->LoadPipeline(hwnd);
    motor->LoadAssets();

    game = std::shared_ptr<Game>(new Game(motor));

    try
    {
        game->Init(hwnd);
    }
    catch (const std::exception& e)
    {
        motor.reset();
    }
    // Run the message loop.
    MSG msg = { };
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ACTIVATEAPP:
        Keyboard::ProcessMessage(uMsg, wParam, lParam);
        Mouse::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        Keyboard::ProcessMessage(uMsg, wParam, lParam);
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;

    case WM_SYSKEYDOWN:
        Keyboard::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        if(motor)
            motor->OnResize(hwnd);
        return 0;
    case WM_PAINT:
        if (motor)
        {
            game->Update();
            motor->OnUpdate();
            motor->OnRender();
        }
        return 0;

    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}