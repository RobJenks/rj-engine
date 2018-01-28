#pragma once

#include <vector>
#include "IntVector.h"
#include "Shaders.h"
#include "DX11_Core.h"
#include "Shader.h"
#include "ShaderParameterDX11.h"
#include "ShaderMacros.h"
#include "InputLayoutDesc.h"

class ShaderDX11 : public Shader
{
public:

	typedef std::vector<ShaderParameterDX11>						ShaderParameterSet;
	typedef ShaderParameterSet::size_type							ShaderParameterIndex;
	typedef std::map<std::string, ShaderParameterIndex>				ShaderParameterMapping;
	static ShaderParameterIndex										INVALID_SHADER_PARAMETER;

	ShaderDX11(void);
	~ShaderDX11(void);

	Shader::Type GetType() const;

	// Load shader data
	bool LoadShaderFromString(	Shader::Type shadertype, const std::string& shaderSource, const std::wstring& sourceFileName, const std::string& entryPoint, 
								const std::string& profile, const InputLayoutDesc *input_layout = NULL);
	bool LoadShaderFromFile(Shader::Type shadertype, const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, const InputLayoutDesc *input_layout = NULL);

	// Retrieve parameter references
	bool							HasParameter(const std::string & name) const; 
	ShaderParameterSet::size_type	GetParameterIndexByName(const std::string& name) const; 
	CMPINLINE ShaderParameterDX11 & GetParameter(ShaderParameterSet::size_type index) { return m_parameters[index]; }

	// Query for the latest supported shader profile
	std::string GetLatestProfile(Shader::Type type) const;

	// Specific slot number for key rendering data
	CMPINLINE SlotID				GetMaterialSlot(void) const { return m_slot_material; }
	CMPINLINE void					SetMaterialSlot(SlotID slot_id) { m_slot_material = slot_id; }


	// Dispatch a compute shader using the specified thread group configuration
	void Dispatch(const UINTVECTOR3 & numGroups);

	// Bind or unbind shader from the current rendering context
	void Bind(void);
	void Unbind(void);

	// Validate the shader resource bindings and report any errors
	bool ValidateShader(void);

private:

	// Release any compiled shaders (during destruction or if we are loading a new shader)
	void ReleaseShaders(void);

private:

	Shader::Type						m_type;

	// One pointer will be populated based on the shader type
	ID3D11VertexShader *				m_vs;
	ID3D11PixelShader *					m_ps;
	ID3D11HullShader *					m_hs;
	ID3D11DomainShader *				m_ds;
	ID3D11GeometryShader * 				m_gs;
	ID3D11ComputeShader	*				m_cs;

	// Key shader parameters
	ShaderParameterSet					m_parameters;			// Linear collection of parameters
	ShaderParameterMapping				m_parameter_mapping;	// Mapping from parameter name to parameter set index
	ID3D11InputLayout *					m_inputlayout;
	ID3DBlob * 							m_shaderblob;
	std::wstring						m_filename; 
	std::string							m_entrypoint;
	std::string							m_profile;
	
	// Slot indices for key data types
	SlotID								m_slot_material;		// Slot for the material constant buffer data

};