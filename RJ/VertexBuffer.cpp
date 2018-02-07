#include "VertexBuffer.h"
#include "../Definitions/ModelData.h"


VertexBuffer::VertexBuffer(const void *data, UINT count, UINT stride) noexcept
	: 
	Buffer(Buffer::BufferType::VertexBuffer, data, count, stride)
{
}

VertexBuffer::VertexBuffer(const ModelData & model_data) noexcept
	:
	Buffer(Buffer::BufferType::VertexBuffer, (const void *)model_data.VertexData, model_data.VertexCount, sizeof(ModelData::TVertex))
{
}


VertexBuffer::~VertexBuffer(void) noexcept
{
}