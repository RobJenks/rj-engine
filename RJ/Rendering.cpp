#include "Rendering.h"
#include "Utility.h"

// Return the name of device & context implementation classes
std::string Rendering::GetRenderDeviceTypeName(void)
{
	return MSTRING(RENDER_DEVICE_TYPE);
}

std::string Rendering::GetRenderDeviceContextTypeName(void)
{
	return MSTRING(RENDER_CONTEXT_TYPE);
}

std::string Rendering::GetSwapChainInterfaceTypeName(void)
{
	return MSTRING(SWAP_CHAIN_TYPE);
}

std::string Rendering::GetDXGIFactoryTypeName(void)
{
	return MSTRING(DXGI_FACTORY_TYPE);
}


