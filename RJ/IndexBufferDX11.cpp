#include "IndexBufferDX11.h"
#include "CoreEngine.h"


// Construct an empty buffer
IndexBufferDX11::IndexBufferDX11(void) noexcept
	:
	IndexBuffer()
{
}

// Construct an index buffer with the specified data
IndexBufferDX11::IndexBufferDX11(const void *data, UINT count, UINT stride) noexcept
	:
	IndexBuffer(data, count, stride)
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