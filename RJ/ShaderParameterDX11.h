#pragma once

#include <string>
#include "CompilerSettings.h"
#include "ShaderParameter.h"
#include "Shader.h"
class ConstantBufferDX11;
class StructuredBufferDX11;
class TextureDX11;
class SamplerStateDX11;


class ShaderParameterDX11 : ShaderParameter
{
public:

	// Constructor to create a new parameter with the given details
	ShaderParameterDX11(Type type,  const std::string & name, Shader::Type shader_target_type, Shader::SlotID shader_target_slotid);

	// Return basic details on the shader parameter
	CMPINLINE ShaderParameter::Type		GetType(void) const { return m_type; }
	CMPINLINE std::string				GetName(void) const { return m_name; }
	CMPINLINE Shader::Type				GetShaderType(void) const { return m_shadertype; }
	CMPINLINE Shader::SlotID			GetSlotID(void) const { return m_slotid; }

	// Assign a resource to this shader parameter
	void								Set(ConstantBufferDX11 *buffer);
	void								Set(StructuredBufferDX11 *buffer);
	void								Set(TextureDX11 *texture);
	void								Set(SamplerStateDX11 *sampler);

	// Bind the parameter to a given slot in the specified shader type
	void								Bind(void);

	// Unbind the (any) resource from this shader slot
	void								Unbind(void);

	// Validate the content of this shader parameter
	bool								Validate(void);

private:

	ShaderParameter::Type				m_type;
	std::string							m_name;
	Shader::Type						m_shadertype;
	Shader::SlotID						m_slotid;

	// Possible resource pointers; determined by m_type
	ConstantBufferDX11 *				m_cbuffer;
	StructuredBufferDX11 *				m_sbuffer;
	TextureDX11 *						m_texture;
	SamplerStateDX11 *					m_sampler;


};