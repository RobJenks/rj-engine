#include "StructuredBuffer.h"
#include "DX11_Core.h"


StructuredBuffer::StructuredBuffer(const D3D11_BUFFER_DESC & buffer_desc, const void *data, UINT count, UINT stride)
	:
	Buffer(Buffer::BufferType::StructuredBuffer, data, count, stride)
{
}
