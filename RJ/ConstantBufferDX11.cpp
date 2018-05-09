#include "ConstantBufferDX11.h"
#include "DX11_Core.h"
#include "CoreEngine.h"



// Constructor; create a new constant buffer of the given size
ConstantBufferDX11::ConstantBufferDX11(UINT buffer_size)
	:
	ConstantBuffer(buffer_size)
{
}

// Bind this resource to the given shader target
void ConstantBufferDX11::Bind(Shader::Type shadertype, Shader::SlotID slot_id) const
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
void ConstantBufferDX11::Unbind(Shader::Type shadertype, Shader::SlotID slot_id) const
{
	switch (shadertype)
	{
		case Shader::Type::VertexShader:
			Game::Engine->GetDeviceContext()->VSSetConstantBuffers(slot_id, 1, null_buffer);
			break;
		case Shader::Type::PixelShader:
			Game::Engine->GetDeviceContext()->PSSetConstantBuffers(slot_id, 1, null_buffer);
			break;
		case Shader::Type::HullShader:
			Game::Engine->GetDeviceContext()->HSSetConstantBuffers(slot_id, 1, null_buffer);
			break;
		case Shader::Type::DomainShader:
			Game::Engine->GetDeviceContext()->DSSetConstantBuffers(slot_id, 1, null_buffer);
			break;
		case Shader::Type::GeometryShader:
			Game::Engine->GetDeviceContext()->GSSetConstantBuffers(slot_id, 1, null_buffer);
			break;
		case Shader::Type::ComputeShader:
			Game::Engine->GetDeviceContext()->CSSetConstantBuffers(slot_id, 1, null_buffer);
			break;
	}
}

// Default destructor
ConstantBufferDX11::~ConstantBufferDX11(void)
{
	ReleaseIfExists(m_buffer[0]);
}

