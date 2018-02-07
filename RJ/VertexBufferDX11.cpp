#include "VertexBufferDX11.h"
#include "CoreEngine.h"
#include "../Definitions/ModelData.h"


VertexBufferDX11::VertexBufferDX11(const void *data, UINT count, UINT stride) noexcept
	:
	VertexBuffer(data, count, stride)
{
}

VertexBufferDX11::VertexBufferDX11(const ModelData & model_data) noexcept
	:
	VertexBuffer(model_data)
{
}



void VertexBufferDX11::Bind(UINT slot_id)
{
	Game::Engine->GetDeviceContext()->IASetVertexBuffers(slot_id, 1, m_buffer, m_stride, m_offset);
}

void VertexBufferDX11::Unbind(UINT slot_id)
{
	Game::Engine->GetDeviceContext()->IASetVertexBuffers(slot_id, 1, null_buffer, nullptr, nullptr);
}



VertexBufferDX11::~VertexBufferDX11(void) noexcept
{
}


