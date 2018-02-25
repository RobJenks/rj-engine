#include <cassert>
#include "Logging.h"
#include "ShaderParameterDX11.h"
#include "ConstantBufferDX11.h"
#include "StructuredBufferDX11.h"
#include "TextureDX11.h"
#include "SamplerStateDX11.h"


// Constructor to create a new parameter with the given details
ShaderParameterDX11::ShaderParameterDX11(Type type, const std::string & name, Shader::Type shader_target_type, Shader::SlotID shader_target_slotid)
	: 
	m_type(type), 
	m_name(name), 
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
void ShaderParameterDX11::Set(SamplerStateDX11 *sampler)
{
	assert(m_type == Type::Sampler);
	assert(sampler != NULL);
	m_sampler = sampler;
}

// Remove any resource currently assigned to the parameter (will not be deallocated, only unmapped)
void ShaderParameterDX11::UnmapResources(void)
{
	m_cbuffer = NULL;
	m_sbuffer = NULL;
	m_texture = NULL;
	m_sampler = NULL;
}

// Bind the parameter to a given slot in the specified shader type
void ShaderParameterDX11::Bind(void)
{
	if (m_cbuffer)
	{
		assert(m_type == Type::ConstantBuffer);
		m_cbuffer->Bind(m_shadertype, m_slotid);
	}
	else if (m_sbuffer)
	{
		assert(m_type == Type::StructuredBuffer);
		m_sbuffer->Bind(m_shadertype, m_slotid, ShaderParameter::Type::StructuredBuffer);
	}
	else if (m_texture)
	{
		assert(m_type == Type::Texture);
		m_texture->Bind(m_shadertype, m_slotid, ShaderParameter::Type::Texture);
	}
	else if (m_sampler)
	{
		assert(m_type == Type::Sampler);
		m_sampler->Bind(m_shadertype, m_slotid);
	}
}

// Unbind the (any) resource from this shader slot
void ShaderParameterDX11::Unbind(void)
{
	switch (m_type)
	{
		case Type::ConstantBuffer:
			m_cbuffer->Unbind(m_shadertype, m_slotid);											
			break;
		case Type::StructuredBuffer:
			m_sbuffer->Unbind(m_shadertype, m_slotid, ShaderParameter::Type::StructuredBuffer);			
			break;
		case Type::Texture:
			m_texture->Unbind(m_shadertype, m_slotid, ShaderParameter::Type::Texture);			
			break;
		case Type::Sampler:
			m_sampler->Unbind(m_shadertype, m_slotid);			
			break;
	}
}

// Validate the content of this shader parameter
bool ShaderParameterDX11::Validate(void)
{
	unsigned int errors = 0U;

	std::vector<std::tuple<ShaderParameter::Type, void**>> parameters = {
		std::make_tuple(ShaderParameter::Type::ConstantBuffer, (void**)&m_cbuffer), 
		std::make_tuple(ShaderParameter::Type::StructuredBuffer, (void**)&m_sbuffer),
		std::make_tuple(ShaderParameter::Type::Texture, (void**)&m_texture),
		std::make_tuple(ShaderParameter::Type::Sampler, (void**)&m_sampler),
	};

	for (auto & entry : parameters)
	{
		if (*(std::get<1>(entry)))				// If the resource pointer is non-null
		{
			if (m_type != std::get<0>(entry))	// But the parameter is not of this type
			{
				++errors;
				Game::Log << LOG_ERROR << "Shader parameter \"" << m_name << "\" has mapped " << ShaderParameter::ParameterTypeToString(std::get<0>(entry))
					<< " resource but is of type " << ShaderParameter::ParameterTypeToString(m_type) << "\n";
			}
		}
		/* Currently disabled, since no parameter types are guaranteed to be non-null on initialisation */
		/*else									// If the resource pointer is null
		{
			if (m_type == std::get<0>(entry))	// But the parameter is of this type
			{
				++errors;
				Game::Log << LOG_ERROR << "Shader parameter \"" << m_name << "\" is of type " << ShaderParameter::ParameterTypeToString(std::get<0>(entry))
					<< " but has no mapped resource of this type\n";
			}
		}*/
	}

	if (errors != 0U)
	{
		Game::Log << LOG_ERROR << "Shader parameter validation for \"" << m_name << "\" failed with " << errors << " errors\n";
		return false;
	}

	return true;
}



