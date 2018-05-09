#include "DynamicVertexBufferDX11.h"


DynamicVertexBufferDX11::DynamicVertexBufferDX11(const void *data, UINT count, UINT stride) noexcept
	:
	VertexBufferDX11
	(
		CreateBufferDescriptor(
			Buffer::DetermineDefaultBindFlags(Buffer::BufferType::VertexBuffer),
			D3D11_USAGE::D3D11_USAGE_DYNAMIC,										// Must be configured for dynamic usage in order for CPU Write access flag to be set
			D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,							// Dynamic VBs also allow CPU write to the underlying resource after instantiation
			Buffer::DetermineDefaultMiscFlags(Buffer::BufferType::VertexBuffer),
			(count * stride),
			0U				
		), 
		data, count, stride
	)
{
}


DynamicVertexBufferDX11::~DynamicVertexBufferDX11(void) noexcept
{
}


