#pragma once

#ifndef __DX11_CoreH__
#define __DX11_CoreH__

// Disable compiler warnings about macro redefiniiton, caused by the use of new Win 8.1 Dev kit alongside the existing
// DirectX SDK content.
#pragma warning( push )
#pragma warning( disable : 4005 )

// Flag to enable DX debug mode, if required.  Only available if we are in application debug mode
#ifdef _DEBUG
#	define D3D_DEBUG_INFO
#endif

// Enable strict access to XMMATRIX structures to prevent inefficient per-component access
#define XM_STRICT_XMMATRIX

// Include core DX11 headers for basic functionality
//#include <d3d11.h>
#include <d3d11_2.h>
#include <DirectXMath.h>
#include <dxgi.h>

// Add compiler commments for the linker to associate with relevant precompiled libraries
#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "directxmath.lib")
#pragma comment(lib, "windowscodecs.lib")

// Define the DXI version if it is not already set; fix to prevent compiler warnings where this is not set correctly in the DX SDK
#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION 0x0800
#endif

// Classes using DirectX functionality are very likely to need 16-bit aligned allocations
#include "ALIGN16.h"

// Use the DirectX namespace as standard
using namespace DirectX;

// Define custom types for the vector and matrix class that are individually 16-bit aligned, to avoid issues with unaligned register access
typedef __declspec(align(16))XMVECTOR AXMVECTOR;	// 16-bit aligned vector class
typedef __declspec(align(16))XMMATRIX AXMMATRIX;	// 16-bit aligned matrix class

// Custom vector type suitable for use in arrays; will ensure that all array elements are themselves 16-bit aligned
typedef __declspec(align(16)) struct AXMVECTOR_P_T : public ALIGN16<AXMVECTOR_P_T> 
{ 
	AXMVECTOR value; 
	AXMVECTOR_P_T(void) { }
	AXMVECTOR_P_T(const XMVECTOR & v) : value(v) { }
} AXMVECTOR_P;

// Custom matrix type suitable for use in arrays; will ensure that all array elements are themselves 16-bit aligned
typedef __declspec(align(16)) struct AXMMATRIX_P_T : public ALIGN16<AXMMATRIX_P_T> { AXMMATRIX value; } AXMMATRIX_P;

// Re-enable compiler warnings for macro redefinition, once all DX headers have been processed
#pragma warning( pop )


#endif