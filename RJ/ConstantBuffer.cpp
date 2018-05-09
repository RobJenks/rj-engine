#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(UINT buffer_size)
	:
	Buffer(Buffer::BufferType::ConstantBuffer, NULL, 1U, buffer_size)
{
}