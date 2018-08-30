#pragma once

#include <vector>
#include "ErrorCodes.h"
#include "IntVector.h"
#include "Shaders.h"
#include "DX11_Core.h"
#include "Shader.h"
#include "ShaderParameterDX11.h"
#include "ShaderMacros.h"
#include "InputLayoutDesc.h"
struct GameConsoleCommand;

// Debug flag which, when set, will silently ignore shader data bindings to invalid parameters.  Allows more
// in-depth shader debugging since it does not matter if parameters are optimised out by the HLSL compiler
// between hot-loads.  Only allowed during debug builds
#define DEBUG_SHADER_RELAXED_BINDINGS


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
	bool LoadShaderFromString(  const std::string & name, Shader::Type shadertype, const std::string& shaderSource, const std::wstring& sourceFileName, const std::string& entryPoint,
								const std::string& profile, const InputLayoutDesc *input_layout = NULL, 
								const ShaderMacros::MacroData & macros = ShaderMacros::NONE);
	bool LoadShaderFromFile(const std::string & name, Shader::Type shadertype, const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, 
							const InputLayoutDesc *input_layout = NULL, const ShaderMacros::MacroData & macros = ShaderMacros::NONE);

	// Initialise any shader parameters that can be assigned prior to rendering
	Result							InitialisePreAssignableParameters(void);

	// Retrieve parameter references
	bool							HasParameter(const std::string & name) const; 
	ShaderParameterSet::size_type	GetParameterIndexByName(const std::string& name) const; 
	CMPINLINE ShaderParameterDX11 & GetParameter(ShaderParameterSet::size_type index) { return m_parameters[index]; }

	// Convenience method for setting the data bound to the given shader parameter index
	// Also allows relaxed binding restrictions for shader debugging
	template <typename T>
	void							SetParameterData(ShaderParameterSet::size_type index, T *data)
	{
#if defined(_DEBUG) && defined(DEBUG_SHADER_RELAXED_BINDINGS)
		if (index == ShaderDX11::INVALID_SHADER_PARAMETER) return;
#endif

		m_parameters[index].Set(data);
	}

	// Query for the latest supported shader profile
	std::string						GetLatestProfile(Shader::Type type) const;

	// Specific slot number for key rendering data
	CMPINLINE SlotID				GetMaterialSlot(void) const { return m_slot_material; }
	CMPINLINE void					SetMaterialSlot(SlotID slot_id) { m_slot_material = slot_id; }

	// Attempt to reload the shader from disk
	Result							Reload(void);

	// Dispatch a compute shader using the specified thread group configuration
	void Dispatch(const UINTVECTOR3 & numGroups);

	// Bind or unbind shader from the current rendering context
	void Bind(void);
	void Unbind(void);

	// Validate the shader resource bindings and report any errors
	bool ValidateShader(void);

	// Shader macros
	std::string						GetShaderMacroString(void) const;
	void							SetMacro(const std::string & key, const std::string & value);
	void							DeleteMacro(const std::string & key);
	void							ClearMacros(void);

	// Process a debug command that was propogated from the render asset manager
	bool							ProcessCommand(GameConsoleCommand & command);


private:

	// Unmap any resources currently assigned to parameter slots.  Use primarily when re-loading shaders to avoid
	// any leftover pointers being used inadvertantly before they are re-assigned (should never happen, but to be safe)
	void							UnmapAllParameters(void);

	// Release any compiled shaders (during destruction or if we are loading a new shader)
	void							ReleaseShaders(void);

	// Release all resources, including any compiled shaders
	void							ReleaseAllResources(void);

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
	std::string							m_name;					// Unique per shader
	ShaderParameterSet					m_parameters;			// Linear collection of parameters
	ShaderParameterMapping				m_parameter_mapping;	// Mapping from parameter name to parameter set index
	const InputLayoutDesc *				m_inputlayout_desc;		// Application description of the shader input layout (for VS only)
	ID3D11InputLayout *					m_inputlayout;			// Compiled D3D input layout (for VS only)
	ID3DBlob * 							m_shaderblob;
	std::wstring						m_filename; 
	std::string							m_entrypoint;
	std::string							m_profile;
	ShaderMacros::MacroData				m_macros;
	
	// Slot indices for key data types
	SlotID								m_slot_material;		// Slot for the material constant buffer data

};