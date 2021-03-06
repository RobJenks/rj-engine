#include <vector>
#include "IndexBuffer.h"

// Construct an index buffer with the specified data
IndexBuffer::IndexBuffer(const void *data, UINT count, UINT stride) noexcept
	:
	Buffer(Buffer::BufferType::IndexBuffer, data, count, stride)
{
}

// Construct an index buffer based on the given model data
IndexBuffer::IndexBuffer(const ModelData & model_data) noexcept
	:
	Buffer(Buffer::BufferType::IndexBuffer, (const void *)model_data.IndexData, model_data.IndexCount, sizeof(IndexBuffer::INDEX_TYPE))
{
}

// Creates a sequential index buffer of the given length, using the default index format
IndexBuffer IndexBuffer::CreateSequentialIndexBuffer(UINT count)
{
	assert(count > 0U);
	
	std::vector<IndexBuffer::INDEX_TYPE> data;
	data.reserve(count);

	for (UINT i = 0U; i < count; ++i)
	{
		data.push_back(i);
	}

	return IndexBuffer((const void*)data.data(), count, sizeof(IndexBuffer::INDEX_TYPE));
}

IndexBuffer::~IndexBuffer(void) noexcept
{

}