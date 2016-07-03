#include "pch.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace Library;
using namespace Rendering;
using namespace std;

void InitializeWindow(HINSTANCE instance, const wstring& className, const wstring windowTitle, int showCommand);
LRESULT WINAPI WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
POINT CenterWindow(int windowWidth, int windowHeight);
void Shutdown(const wstring& className);

UINT mRenderTargetWidth = 1024;
UINT mRenderTargetHeight = 768;

HWND mWindowHandle;
WNDCLASSEX mWindow;

unique_ptr<RenderingGame> mGame;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	UNREFERENCED_PARAMETER(previousInstance);
	UNREFERENCED_PARAMETER(commandLine);

	SetCurrentDirectory(UtilityWin32::ExecutableDirectory().c_str());

	ThrowIfFailed(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED), "Error initializing COM.");

	wstring windowClassName = L"RenderingClass";
	wstring windowTitle = L"DirectX Essentials";

	InitializeWindow(instance, windowClassName, windowTitle, showCommand);

	auto getRenderTargetSize = [](SIZE& renderTargetSize)
	{
		renderTargetSize.cx = mRenderTargetWidth;
		renderTargetSize.cy = mRenderTargetHeight;
	};

	auto getWindow = [&]() -> void*
	{
		return reinterpret_cast<void*>(mWindowHandle);
	};

	mGame = make_unique<RenderingGame>(getWindow, getRenderTargetSize);
	mGame->UpdateRenderTargetSize();
	mGame->Initialize();

	MSG message = { 0 };

	try
	{
		while (message.message != WM_QUIT)
		{
			if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{
				mGame->Run();
			}
		}
	}
	catch (GameException ex)
	{
		MessageBox(mWindowHandle, ex.whatw().c_str(), windowTitle.c_str(), MB_ABORTRETRYIGNORE);
	}

	Shutdown(windowClassName);

	CoUninitialize();

	return static_cast<int>(message.wParam);
}

void InitializeWindow(HINSTANCE instance, const wstring& className, const wstring windowTitle, int showCommand)
{
	ZeroMemory(&mWindow, sizeof(mWindow));
	mWindow.cbSize = sizeof(WNDCLASSEX);
	mWindow.style = CS_CLASSDC;
	mWindow.lpfnWndProc = WndProc;
	mWindow.hInstance = instance;
	mWindow.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	mWindow.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	mWindow.hCursor = LoadCursor(nullptr, IDC_ARROW);
	mWindow.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	mWindow.lpszClassName = className.c_str();

	RECT windowRectangle = { 0, 0, static_cast<LONG>(mRenderTargetWidth), static_cast<LONG>(mRenderTargetHeight) };
	AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

	RegisterClassEx(&mWindow);
	POINT center = CenterWindow(mRenderTargetWidth, mRenderTargetHeight);
	mWindowHandle = CreateWindow(className.c_str(), windowTitle.c_str(), WS_OVERLAPPEDWINDOW, center.x, center.y, windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, nullptr, nullptr, instance, nullptr);

	ShowWindow(mWindowHandle, showCommand);
	UpdateWindow(mWindowHandle);
}

LRESULT WINAPI WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
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
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	}

	return DefWindowProc(windowHandle, message, wParam, lParam);
}

POINT CenterWindow(int windowWidth, int windowHeight)
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	POINT center;
	center.x = (screenWidth - windowWidth) / 2;
	center.y = (screenHeight - windowHeight) / 2;

	return center;
}

void Shutdown(const wstring& className)
{
	mGame->Shutdown();
	UnregisterClass(className.c_str(), mWindow.hInstance);
}