#include "VertexBuffer.h"
#include "../Definitions/ModelData.h"



VertexBuffer::VertexBuffer(void)
{
	m_buffertype = Buffer::BufferType::VertexBuffer;
}

VertexBuffer::VertexBuffer(const void *data, UINT count, UINT stride)
	: 
	Buffer(Buffer::BufferType::VertexBuffer, data, count, stride)
{
}

VertexBuffer::VertexBuffer(const ModelData & model_data)
	:
	Buffer(Buffer::BufferType::VertexBuffer, (const void *)model_data.VertexData, model_data.VertexCount, sizeof(ModelData::TVertex))
{
}


VertexBuffer::~VertexBuffer(void)
{

}