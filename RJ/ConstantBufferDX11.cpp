#include "ConstantBufferDX11.h"
#include "DX11_Core.h"
#include "CoreEngine.h"


// Static null buffer resource, used for more efficient unbinding
ID3D11Buffer * const ConstantBufferDX11::m_null_buffer[1] = { nullptr };


// Constructor; create a new constant buffer of the given size
ConstantBufferDX11::ConstantBufferDX11(UINT buffer_size)
	:
	m_buffersize(buffer_size)
{
	assert(buffer_size > 0);

	D3D11_BUFFER_DESC bufferdesc;
	memset(&bufferdesc, 0, sizeof(D3D11_BUFFER_DESC));

	bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferdesc.ByteWidth = m_buffersize;
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;

	m_buffer[0] = NULL;
	HRESULT result = Game::Engine->GetDevice()->CreateBuffer(&bufferdesc, NULL, &(m_buffer[0]));
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Failed to create constant buffer resource (hr: " << result << ", size: " << m_buffersize << ")\n";
	}
}

// Map data into this constant buffer
void ConstantBufferDX11::Set(const void *data, UINT data_size)
{
	assert(data_size == m_buffersize);
	auto devicecontext = Game::Engine->GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedresource;
	if (FAILED(devicecontext->Map(m_buffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedresource)))
	{
		return;
	}

	memcpy(mappedresource.pData, data, m_buffersize);
	devicecontext->Unmap(m_buffer[0], 0);
}

// Bind this resource to the given shader target
void ConstantBufferDX11::Bind(Shader::Type shadertype, Shader::SlotID slot_id)
{
	switch (shadertype)
	{
		case Shader::Type::VertexShader:
			Game::Engine->GetDeviceContext()->VSSetConstantBuffers(slot_id, 1, m_buffer);		
			break;
		case Shader::Type::PixelShader:
			Game::Engine->GetDeviceContext()->PSSetConstantBuffers(slot_id, 1, m_buffer);
			break;
		case Shader::Type::HullShader:
			Game::Engine->GetDeviceContext()->HSSetConstantBuffers(slot_id, 1, m_buffer);
			break;
		case Shader::Type::DomainShader:
			Game::Engine->GetDeviceContext()->DSSetConstantBuffers(slot_id, 1, m_buffer);
			break;
		case Shader::Type::GeometryShader:
			Game::Engine->GetDeviceContext()->GSSetConstantBuffers(slot_id, 1, m_buffer);
			break;
		case Shader::Type::ComputeShader:
			Game::Engine->GetDeviceContext()->CSSetConstantBuffers(slot_id, 1, m_buffer);
			break;
	}
}

// Remove this (or any) binding from the given shader target
void ConstantBufferDX11::Unbind(Shader::Type shadertype, Shader::SlotID slot_id)
{
	switch (shadertype)
	{
		case Shader::Type::VertexShader:
			Game::Engine->GetDeviceContext()->VSSetConstantBuffers(slot_id, 1, m_null_buffer);
			break;
		case Shader::Type::PixelShader:
			Game::Engine->GetDeviceContext()->PSSetConstantBuffers(slot_id, 1, m_null_buffer);
			break;
		case Shader::Type::HullShader:
			Game::Engine->GetDeviceContext()->HSSetConstantBuffers(slot_id, 1, m_null_buffer);
			break;
		case Shader::Type::DomainShader:
			Game::Engine->GetDeviceContext()->DSSetConstantBuffers(slot_id, 1, m_null_buffer);
			break;
		case Shader::Type::GeometryShader:
			Game::Engine->GetDeviceContext()->GSSetConstantBuffers(slot_id, 1, m_null_buffer);
			break;
		case Shader::Type::ComputeShader:
			Game::Engine->GetDeviceContext()->CSSetConstantBuffers(slot_id, 1, m_null_buffer);
			break;
	}
}

// Default destructor
ConstantBufferDX11::~ConstantBufferDX11(void)
{
}

