#pragma once

#ifndef __DX11_CoreH__
#define __DX11_CoreH__

// Disable compiler warnings about macro redefiniiton, caused by the use of new Win 8.1 Dev kit alongside the existing
// DirectX SDK content.
#pragma warning( disable : 4005 )

// Flag to enable DX debug mode, if required.  Only available if we are in application debug mode
#ifdef _DEBUG
#	define D3D_DEBUG_INFO
#endif

// Enable strict access to XMMATRIX structures to prevent inefficient per-component access
#define XM_STRICT_XMMATRIX

// Include core DX11 headers for basic functionality
#include <d3d11.h>
#include <DirectXMath.h>

// Add compiler commments for the linker to associate with relevant precompiled libraries
//#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "directxmath.lib")
#pragma comment(lib, "windowscodecs.lib")

// Define the DXI version if it is not already set; fix to prevent compiler warnings where this is not set correctly in the DX SDK
#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION 0x0800
#endif

// Classes using DirectX functionality are very likely to need 16-bit aligned allocations
#include "ALIGN16.h"

// Use the DirectX namespace as standard
using namespace DirectX;

// Re-enable compiler warnings for macro redefinition, once all DX headers have been processed
#pragma warning( default : 4005 )


#endif