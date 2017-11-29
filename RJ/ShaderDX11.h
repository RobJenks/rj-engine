#pragma once

#include <vector>
#include "Shaders.h"
#include "DX11_Core.h"
#include "Shader.h"
#include "ShaderParameterDX11.h"

class ShaderDX11 : Shader
{
public:

	// Bind shader to the current rendering context
	void Bind(void);



private:

	Shader::Type						m_type;
	ID3D11InputLayout *					m_inputlayout;
	std::vector<ShaderParameterDX11>	m_parameters;


	// One pointer will be populated based on the shader type
	ID3D11VertexShader *				m_vs;
	ID3D11PixelShader *					m_ps;
	ID3D11HullShader *					m_hs;
	ID3D11DomainShader *				m_ds;
	ID3D11GeometryShader * 				m_gs;
	ID3D11ComputeShader	*				m_cs;

};