#pragma once

#include <cstdint>


enum class ClearFlags : uint8_t
{
	Colour = (1 << 0),
	Depth = (1 << 1), 
	Stencil = (1 << 2), 
	
	DepthStencil = (Depth | Stencil), 
	All = (Colour | Depth | Stencil)
};