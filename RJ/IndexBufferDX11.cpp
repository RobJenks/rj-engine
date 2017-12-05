#include "IndexBufferDX11.h"
#include "CoreEngine.h"

IndexBufferDX11::IndexBufferDX11(void)
	:
	IndexBuffer()
{
}

IndexBufferDX11::IndexBufferDX11(const void *data, UINT count, UINT stride)
	:
	IndexBuffer(data, count, stride)
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