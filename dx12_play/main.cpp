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
    std::string s1 = pCmdLine;


    std::cout << s1 << std::endl;;

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
        L"D12",    // Window text
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
    game->Init(s1);

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
            motor->OnUpdate();
            motor->OnRender();
        }
        return 0;

    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}