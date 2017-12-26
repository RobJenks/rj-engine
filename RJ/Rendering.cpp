#include "Rendering.h"
#include "Utility.h"

// Return the name of device & context implementation classes
std::string Rendering::GetRenderDeviceTypeName(void)
{
	return STRING(RENDER_DEVICE_TYPE);
}

std::string Rendering::GetRenderDeviceContextTypeName(void)
{
	return STRING(RENDER_CONTEXT_TYPE);
}

std::string Rendering::GetSwapChainInterfaceTypeName(void)
{
	return STRING(SWAP_CHAIN_TYPE);
}
