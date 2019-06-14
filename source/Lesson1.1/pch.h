#pragma once

// Windows
#include <windows.h>
#include <winrt\Windows.Foundation.h>

// Standard
#include <string>
#include <string>
#include <exception>

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

// Guidelines Support Library
#include <gsl\gsl>

// DirectX
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>