#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(void)
{
	m_buffertype = Buffer::BufferType::IndexBuffer;
}

IndexBuffer::IndexBuffer(const void *data, UINT count, UINT stride)
	:
	Buffer(Buffer::BufferType::IndexBuffer, data, count, stride)
{
}

IndexBuffer::~IndexBuffer(void)
{

}