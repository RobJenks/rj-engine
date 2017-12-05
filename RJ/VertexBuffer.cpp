#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(void)
{
	m_buffertype = Buffer::BufferType::VertexBuffer;
}

VertexBuffer::VertexBuffer(const void *data, UINT count, UINT stride)
	: 
	Buffer(Buffer::BufferType::VertexBuffer, data, count, stride)
{
}

VertexBuffer::~VertexBuffer(void)
{

}