#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(void)
{
	m_buffertype = Buffer::BufferType::ConstantBuffer;
	m_buffer_elementcount[0] = 1U;							// Constant buffers are always exactly one element
}