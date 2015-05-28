#pragma once

#ifndef __CompilerSettingsH__
#define __CompilerSettingsH__

#include <d3dcommon.h>

// Define inlining compiler command (same definition as that of D3DXINLINE from DX9)
#define CMPINLINE __forceinline

// Compiler support test for C++ 11
#if __cplusplus > 199711L
#	define RJ_CPP11_SUPPORTED
#else
#	define RJ_NO_CPP11_SUPPORT
#endif

// DirectX feature levels; define lower feature levels for development on non-DX11 compliant hardware
#define REDUCE_DX_FEATURE_LEVEL




#endif