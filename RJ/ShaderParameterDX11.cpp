#include <cassert>
#include "ShaderParameterDX11.h"
#include "ConstantBufferDX11.h"
#include "StructuredBufferDX11.h"
#include "TextureDX11.h"
#include "SamplerDX11.h"


// Constructor to create a new parameter with the given details
ShaderParameterDX11::ShaderParameterDX11(Type type, Shader::Type shader_target_type, Shader::SlotID shader_target_slotid)
	: 
	m_type(type), 
	m_shadertype(shader_target_type), 
	m_slotid(shader_target_slotid), 
	m_cbuffer(NULL), 
	m_sbuffer(NULL), 
	m_texture(NULL), 
	m_sampler(NULL)
{
}

// Assign a resource to this shader parameter
void ShaderParameterDX11::Set(ConstantBufferDX11 *buffer)
{
	assert(m_type == Type::ConstantBuffer);
	assert(buffer != NULL);
	m_cbuffer = buffer;
}

// Assign a resource to this shader parameter
void ShaderParameterDX11::Set(StructuredBufferDX11 *buffer)
{
	assert(m_type == Type::StructuredBuffer);
	assert(buffer != NULL);
	m_sbuffer = buffer;
}

// Assign a resource to this shader parameter
void ShaderParameterDX11::Set(TextureDX11 *texture)
{
	assert(m_type == Type::Texture);
	assert(texture != NULL);
	m_texture = texture;
}

// Assign a resource to this shader parameter
void ShaderParameterDX11::Set(SamplerDX11 *sampler)
{
	assert(m_type == Type::Sampler);
	assert(sampler != NULL);
	m_sampler = sampler;
}

// Bind the parameter to a given slot in the specified shader type
void ShaderParameterDX11::Bind(void)
{
	switch (m_type)
	{
		case Type::ConstantBuffer:
			m_cbuffer->Bind(m_shadertype, m_slotid);			break;
		case Type::StructuredBuffer:
			m_sbuffer->Bind(m_shadertype, m_slotid);			break;
		case Type::Texture:
			m_texture->Bind(m_shadertype, m_slotid);			break;
		case Type::Sampler:
			m_sampler->Bind(m_shadertype, m_slotid);			break;
	}
}

// Unbind the (any) resource from this shader slot
void ShaderParameterDX11::Unbind(void)
{
	switch (m_type)
	{
		case Type::ConstantBuffer:
			m_cbuffer->Unbind(m_shadertype, m_slotid);			break;
		case Type::StructuredBuffer:
			m_sbuffer->Unbind(m_shadertype, m_slotid);			break;
		case Type::Texture:
			m_texture->Unbind(m_shadertype, m_slotid);			break;
		case Type::Sampler:
			m_sampler->Unbind(m_shadertype, m_slotid);			break;
	}
}

