#pragma once

#include <string>

struct ID3D11Device2;
struct ID3D11DeviceContext2;
struct IDXGISwapChain2;


class Rendering
{
public:

	// The device and context type in use for rendering.  Keep decoupled from core engine or renderer 
	// to simplify component dependencies
#	define RENDER_DEVICE_TYPE		ID3D11Device2
#	define RENDER_CONTEXT_TYPE		ID3D11DeviceContext2
#	define SWAP_CHAIN_TYPE			IDXGISwapChain2

	// Type definitions for use in the core engine
	typedef RENDER_DEVICE_TYPE		RenderDeviceType;
	typedef RENDER_CONTEXT_TYPE		RenderDeviceContextType;
	typedef SWAP_CHAIN_TYPE			SwapChainInterfaceType;

	// Return the name of device & context implementation classes
	static std::string				GetRenderDeviceTypeName(void);
	static std::string				GetRenderDeviceContextTypeName(void);
	static std::string				GetSwapChainInterfaceTypeName(void);

	// Maximum supported render targets.  RT count is capped at 8 in DX11 
	static const size_t				MaxRenderTargets = 8U;

private:

};

