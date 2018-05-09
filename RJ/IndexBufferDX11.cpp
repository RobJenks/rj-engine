#include "IndexBufferDX11.h"
#include "CoreEngine.h"


// Construct an index buffer with the specified data
IndexBufferDX11::IndexBufferDX11(const void *data, UINT count, UINT stride) noexcept
	:
	IndexBuffer(data, count, stride)
{
}

// Construct an index buffer based on the given model data
IndexBufferDX11::IndexBufferDX11(const ModelData & model_data) noexcept
	:
	IndexBuffer(model_data)
{
}

// Construct a sequential index buffer of the specified length, using the default index format
IndexBufferDX11::IndexBufferDX11(UINT count) noexcept
	:
	IndexBuffer(std::move(IndexBuffer::CreateSequentialIndexBuffer(count)))
{
}


void IndexBufferDX11::Bind(UINT slot_id)
{
	Game::Engine->GetDeviceContext()->IASetIndexBuffer(m_buffer[0], INDEX_FORMAT, 0);
}

void IndexBufferDX11::Unbind(UINT slot_id)
{
	Game::Engine->GetDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}

IndexBufferDX11::~IndexBufferDX11(void) noexcept
{
}