#include <d3dcompiler.h>
#include <filesystem>
#include <limits>
#include "ShaderDX11.h"
#include "Utility.h"
#include "Logging.h"
#include "DX11_Core.h"
#include "CoreEngine.h"
#include "GameConsoleCommand.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"


// TODO: VS2017 is still implementing as exp branch; convert to std library once available
namespace fs = std::experimental::filesystem;

// Initialise static data
ShaderDX11::ShaderParameterIndex ShaderDX11::INVALID_SHADER_PARAMETER = (std::numeric_limits<ShaderParameterIndex>::max)();


ShaderDX11::ShaderDX11(void)
	: 
	m_name(NullString), 
	m_type(Shader::Type::SHADER_TYPE_COUNT), 
	m_vs(NULL), m_ps(NULL), m_gs(NULL), m_hs(NULL), m_ds(NULL), m_cs(NULL), 
	m_inputlayout_desc(NULL), 
	m_inputlayout(NULL), 
	m_shaderblob(NULL), 
	m_slot_material(Shader::NO_SLOT_ID)
{
}

ShaderDX11::~ShaderDX11()
{
	ReleaseAllResources();
}

void ShaderDX11::ReleaseAllResources()
{
	ReleaseShaders();

	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_shaderblob);
}

void ShaderDX11::ReleaseShaders()
{
	ReleaseIfExists(m_vs);
	ReleaseIfExists(m_ps);
	ReleaseIfExists(m_gs);
	ReleaseIfExists(m_hs);
	ReleaseIfExists(m_ds);
	ReleaseIfExists(m_cs);
}

Shader::Type ShaderDX11::GetType() const
{
	return m_type;
}


bool ShaderDX11::LoadShaderFromString(  const std::string & name, Shader::Type shadertype, const std::string& shaderSource, const std::wstring& sourceFileName,
										const std::string& entryPoint, const std::string& profile, const InputLayoutDesc *input_layout, 
										const ShaderMacros::MacroData & macros)
{
	auto device = Game::Engine->GetDevice();
	HRESULT hr;
	{
		ID3DBlob *shaderBlob;
		ID3DBlob *errorBlob;

		// Determine appropriate shader profile
		std::string _profile = profile;
		if (profile == "latest")
		{
			_profile = GetLatestProfile(shadertype);
			if (_profile.empty())
			{
				Game::Log << LOG_ERROR << "Invalid shader type " << (int)shadertype << " for \"" << name << 
					"\", could not determine supported feature level (" << ConvertWStringToString(sourceFileName) << ")\n";
				return false;
			}
		}

		// Set any shader macros before compilation.  We can directly overwrite any existing data.  Then compile & retrieve the data
		ShaderManager::GetGlobalShaderMacros().ReplaceMacros(macros);
		const D3D_SHADER_MACRO * compiled_macros = ShaderManager::GetGlobalShaderMacros().GetCompiledData();

		// Compilation flags
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if		defined( _DEBUG )
			flags |= D3DCOMPILE_DEBUG;
#		endif

		// Attempt to load and compile the HLSL
		fs::path filePath(sourceFileName);
		std::string sourceFilePath = filePath.string();

		// Enclosed in preprocessor defines to force the use of HLSL definitions during inline C++ compilation, rather than
		// the C++ defines which woul otherwise be used since __cplusplus is defined here
#		define RJ_COMPILING_HLSL
		hr = D3DCompile((LPCVOID)shaderSource.c_str(), shaderSource.size(), sourceFilePath.c_str(), compiled_macros, 
			D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), _profile.c_str(), flags, 0, &shaderBlob, &errorBlob);
#		undef RJ_COMPILING_HLSL

		// Check compilation results and report any errors
		if (FAILED(hr))
		{
			Game::Log << LOG_ERROR << "Compilation of shader \"" << name << "\" failed (hr=" << hr << ", file=" << ConvertWStringToString(sourceFileName) << ")\n";
			if (errorBlob)
			{
				Game::Log << LOG_ERROR << "Error buffer: " << (static_cast<char*>(errorBlob->GetBufferPointer())) << "\n";
			}
			else
			{
				Game::Log << LOG_ERROR << "No further error details available; no error buffer generated\n";
			}
			return false;
		}
		else
		{
			Game::Log << LOG_INFO << "Shader \"" << name << "\" compiled successfully\n";
		}

		m_shaderblob = shaderBlob;
	}

	// Release any existing shader since we are loading a new one
	ReleaseShaders();

	// Store the new shader type and create it now
	m_type = shadertype;
	switch (m_type)
	{
		case Shader::Type::VertexShader:
			hr = device->CreateVertexShader(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), nullptr, &m_vs);
			break;
		case Shader::Type::HullShader:
			hr = device->CreateHullShader(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), nullptr, &m_hs);
			break;
		case Shader::Type::DomainShader:
			hr = device->CreateDomainShader(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), nullptr, &m_ds);
			break;
		case Shader::Type::GeometryShader:
			hr = device->CreateGeometryShader(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), nullptr, &m_gs);
			break;
		case Shader::Type::PixelShader:
			hr = device->CreatePixelShader(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), nullptr, &m_ps);
			break;
		case Shader::Type::ComputeShader:
			hr = device->CreateComputeShader(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), nullptr, &m_cs);
			break;
		default:
			Game::Log << LOG_ERROR << "Invalid shader type " << (int)shadertype << " for " << name << "; cannnot create compiled shader object (" << ConvertWStringToString(sourceFileName )<< ")\n";
			return false;
	}

	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Failed to create shader \"" << name << "\" from compiled source (hr=" << hr << ", file=" << ConvertWStringToString(sourceFileName) << ")\n";
		return false;
	}

	// Use D3D reflection to retrieve parameter details from the shader
	ID3D11ShaderReflection *reflect;
	hr = D3DReflect(m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect);
	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Failed to reflect shader parameters for \"" << name << "\" (hr=" << hr << ")\n";
		return false;
	}

	D3D11_SHADER_DESC shaderdesc;
	hr = reflect->GetDesc(&shaderdesc);
	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Failed to retrieve shader descriptor for \"" << name << "\" (hr=" << hr << ")\n";
		return false;
	}

	/// Can potentially reflect the input layout here via shaderdesc->InputParameters(), though not clear whether instancing is fully-supported

	// We are assigning new shader parameters based on the reflected shader bytecode, so remove any existing parameter data
	m_parameters.clear();

	// Query resource binding sites using the descriptor
	Game::Log << LOG_INFO << "Registering " << shaderdesc.BoundResources << " resource binding sites from \"" << name << "\" shader descriptor\n";
	for (UINT i = 0; i < shaderdesc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflect->GetResourceBindingDesc(i, &bindDesc);
		std::string resourceName = bindDesc.Name;

		ShaderParameter::Type parameterType = ShaderParameter::Type::Unknown;

		switch (bindDesc.Type)
		{
		case D3D_SIT_TEXTURE:
			parameterType = ShaderParameter::Type::Texture;
			break;
		case D3D_SIT_SAMPLER:
			parameterType = ShaderParameter::Type::Sampler;
			break;
		case D3D_SIT_CBUFFER:
			parameterType = ShaderParameter::Type::ConstantBuffer;
			break;
		case D3D_SIT_STRUCTURED:
			parameterType = ShaderParameter::Type::StructuredBuffer;
			break;
		case D3D_SIT_UAV_RWSTRUCTURED:
			parameterType = ShaderParameter::Type::RWBuffer;
			break;
		case D3D_SIT_UAV_RWTYPED:
			parameterType = ShaderParameter::Type::RWTexture;
			break;
		default:
			Game::Log << LOG_WARN << "Encountered unknown shader parameter type of " << (int)parameterType << " for \"" << name << "::" << resourceName << "\" (ix: " << i << ")\n";
		}

		// Record both the parameter and a mapping from (parameter name -> index in m_parameters)
		m_parameters.push_back(ShaderParameterDX11(parameterType, resourceName, shadertype, bindDesc.BindPoint));
		m_parameter_mapping[resourceName] = i;
	}


	// Compile the shader input layout, if applicable
	if (input_layout)
	{
		m_inputlayout_desc = input_layout;
		hr = device->CreateInputLayout(input_layout->Data(), input_layout->ElementCount(), m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), &m_inputlayout);
		if (FAILED(hr))
		{
			Game::Log << LOG_ERROR << "Failed to create input layout for \"" << name << "\" (hr=" << hr << ", file=" << ConvertWStringToString(sourceFileName) << ")\n";
			return false;
		}
	}

	// We have successfully loaded and compiled the shader
	return true;
}

bool ShaderDX11::LoadShaderFromFile(const std::string & name, Shader::Type shadertype, const std::wstring& fileName, const std::string& entryPoint,
									const std::string& profile, const InputLayoutDesc *input_layout, const ShaderMacros::MacroData & macros)
{
	bool result = false;

	fs::path filePath(fileName);
	if (fs::exists(filePath) && fs::is_regular_file(filePath))
	{
		// Store data necessary to reload the shader if it changes on disc.
		m_name = name;
		m_filename = fileName;
		m_entrypoint = entryPoint;
		m_profile = profile;
		m_macros = macros;

		// Load from disk
		std::ifstream inputFile(fileName);
		std::string source((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

		result = LoadShaderFromString(name, shadertype, source, fileName, entryPoint, profile, input_layout, macros);
	}
	else
	{
		Game::Log << LOG_ERROR << "Failed to load shader \"" << name << "\" from external file \"" << ConvertWStringToString(fileName) << "\"; source file may not exist or be otherwise inacessible\n";
	}

	return result;
}

// Initialise any shader parameters that can be assigned prior to rendering
Result ShaderDX11::InitialisePreAssignableParameters(void)
{
	// Process each parameter in turn and attempt to resolve it wherever possible
	for (auto & param : m_parameters)
	{
		switch (param.GetType())
		{
			/* Sampler states can all be pre-assigned */
			case ShaderParameter::Type::Sampler:

				auto * sampler = Game::Engine->GetAssets().GetSamplerState(param.GetName());
				if (!sampler)
				{
					Game::Log << LOG_ERROR << "Cannot locate sampler \"" << param.GetName() << "\" for pre-render initialisation of shader \"" << m_name << "\"\n";
					return ErrorCodes::CannotPreInitialiseShaderParameter;
				}
				param.Set(sampler);
				break;
		}
	}

	// Also store key slot references that we want to avoid finding every frame
	std::vector<std::pair<std::string, Shader::SlotID&>> resolve_slots = {
		{ MaterialBufferName, m_slot_material }
	};

	for (auto & resolve : resolve_slots)
	{
		// Not all shaders will require all parameters, so silently ignore those that are not present
		if (HasParameter(resolve.first))
		{
			auto param_index = GetParameterIndexByName(resolve.first);
			resolve.second = GetParameter(param_index).GetSlotID();
		}
	}

	return ErrorCodes::NoError;
}

// Attempt to reload the shader from disk
Result ShaderDX11::Reload(void)
{
	Game::Log << LOG_INFO << "Reloading shader \"" << m_name << "\" from \"" << ConvertWStringToString(m_filename) << "\"\n";

	// Release any data that will be reloaded
	ReleaseAllResources();

	// Attempt to reload the shader from disk
	bool success = LoadShaderFromFile(m_name, m_type, m_filename, m_entrypoint, m_profile, m_inputlayout_desc, m_macros);
	if (success)
	{
		Game::Log << LOG_INFO << "Reloaded and recompiled shader \"" << m_name << "\"\n";
	}
	else
	{
		Game::Log << LOG_ERROR << "SEVERE: Failed to reload and recompile shader \"" << m_name << "\"\n";
		return ErrorCodes::CouldNotReloadAndRecompileShader;
	}

	// Also attempt to reassign fixed parameter indices, since these may have changed with the signature of the shader
	UnmapAllParameters();
	InitialisePreAssignableParameters();
	Game::Log << LOG_INFO << "Reinitialised shader parameters for shader \"" << m_name << "\"\n";

	// Return success
	return ErrorCodes::NoError;
}


bool ShaderDX11::HasParameter(const std::string & name) const 
{
	return (m_parameter_mapping.find(name) != m_parameter_mapping.end());
}

ShaderDX11::ShaderParameterSet::size_type ShaderDX11::GetParameterIndexByName(const std::string & name) const
{
	ShaderParameterMapping::const_iterator it = m_parameter_mapping.find(name);
	if (it != m_parameter_mapping.end())
	{
		return it->second;
	}
	else
	{
		Game::Log << LOG_WARN << "Attempted to retrieve shader parameter \"" << name << "\" from \"" << m_name << "\"; does not exist\n";
		return INVALID_SHADER_PARAMETER;
	}
}

// Unmap any resources currently assigned to parameter slots.  Use primarily when re-loading shaders to avoid
// any leftover pointers being used inadvertantly before they are re-assigned (should never happen, but to be safe)
void ShaderDX11::UnmapAllParameters(void)
{
	for (auto & param : m_parameters)
	{
		param.UnmapResources();
	}
}

void ShaderDX11::Bind()
{
	auto devicecontext = Game::Engine->GetDeviceContext();

	// Bind all required shader parameters
	for (auto & parameter : m_parameters)
	{
		parameter.Bind();
	}

	// Bind the relevant shader object to the current pipeline context
	if (m_vs)
	{
		devicecontext->IASetInputLayout(m_inputlayout);
		devicecontext->VSSetShader(m_vs, nullptr, 0);
	}
	else if (m_ps)
	{
		devicecontext->PSSetShader(m_ps, nullptr, 0);
	}
	else if (m_gs)
	{
		devicecontext->GSSetShader(m_gs, nullptr, 0);
	}
	else if (m_hs)
	{
		devicecontext->HSSetShader(m_hs, nullptr, 0);
	}
	else if (m_ds)
	{
		devicecontext->DSSetShader(m_ds, nullptr, 0);
	}
	else if (m_cs)
	{
		devicecontext->CSSetShader(m_cs, nullptr, 0);
	}
}

void ShaderDX11::Unbind()
{
	auto devicecontext = Game::Engine->GetDeviceContext();

	// Unbind all shader parameters
	for (auto & parameter : m_parameters)
	{
		parameter.Unbind();
	}

	// Unbind this shader from the current pipeline context
	if (m_vs)
	{
		devicecontext->IASetInputLayout(nullptr);
		devicecontext->VSSetShader(nullptr, nullptr, 0);
	}
	else if (m_hs)
	{
		devicecontext->HSSetShader(nullptr, nullptr, 0);
	}
	else if (m_ds)
	{
		devicecontext->DSSetShader(nullptr, nullptr, 0);
	}
	else if (m_gs)
	{
		devicecontext->GSSetShader(nullptr, nullptr, 0);
	}
	else if (m_ps)
	{
		devicecontext->PSSetShader(nullptr, nullptr, 0);
	}
	else if (m_cs)
	{
		devicecontext->CSSetShader(nullptr, nullptr, 0);
	}
}

// Dispatch a compute shader with the specified thread group configuration
void ShaderDX11::Dispatch(const UINTVECTOR3 & numGroups)
{
	if (m_cs)
	{
		Game::Engine->GetDeviceContext()->Dispatch(numGroups.x, numGroups.y, numGroups.z);
	}
}

// Validate the shader & resource bindings and report any errors
bool ShaderDX11::ValidateShader(void)
{
	/* Verify shader type is valid */
	bool valid_type = true;
	if ((int)m_type < 0 || (int)m_type >= (int)Shader::Type::SHADER_TYPE_COUNT)
	{
		Game::Log << LOG_ERROR << "Shader \"" << m_name << "\" has invalid type: " << (int)m_type << "\n";
		valid_type = false;
	}

	/* Verify shader binary is correctly compiled */
	bool valid_binary = false;
	switch (m_type)
	{
		case Type::ComputeShader:		valid_binary = !!m_cs;		break;
		case Type::DomainShader:		valid_binary = !!m_ds;		break;
		case Type::GeometryShader:		valid_binary = !!m_gs;		break;
		case Type::HullShader:			valid_binary = !!m_hs;		break;
		case Type::PixelShader:			valid_binary = !!m_ps;		break;
		case Type::VertexShader:		valid_binary = !!m_vs;		break;
	}

	if (!valid_binary)
	{
		Game::Log << LOG_ERROR << "Shader \"" << m_name << "\" is of type \"" << Shader::ShaderTypeToString(m_type)
			<< " shader\" but has no compiled " << Shader::ShaderTypeToString(m_type) << " shader resource\n";
	}

	/* Now validate all shader parameters */
	unsigned int invalid_params = 0U;
	size_t n = m_parameters.size();
	for (size_t i = 0U; i < n; ++i)
	{
		// Have the parameter validate its own content.  Parameter will log any errors if they exist
		auto & parameter = m_parameters[i];
		bool valid = parameter.Validate();

		// Also make sure we have a valid mapping for this parameter to the correct parameter index
		auto mappedindex = GetParameterIndexByName(parameter.GetName());
		bool indexvalid = (mappedindex == i);
		if (!indexvalid)
		{
			Game::Log << LOG_ERROR << "Invalid parameter mapping index for \"" << m_name << "::" << parameter.GetName() << 
				"\"; parameter has index " << i << " but is mapped to index " << mappedindex << "\n";
		}

		if (!valid || !indexvalid) ++invalid_params;
	}

	if (invalid_params != 0U)
	{
		Game::Log << LOG_ERROR << "Validation of shader \"" << m_name << "\" failed with " << invalid_params << " invalid parameters\n";
	}

	return ( valid_type && valid_binary && invalid_params == 0U );
}

// Shader macros
std::string ShaderDX11::GetShaderMacroString(void) const
{
	concat str("");
	size_t processed = 0; size_t size = m_macros.size();

	for (const auto & entry : m_macros)
	{
		str(entry.first)("=")(entry.second)((++processed == size) ? "" : ", ");
	}

	return str.str();
}

// Shader macros
void ShaderDX11::SetMacro(const std::string & key, const std::string & value)
{
	m_macros[key] = value;
}

// Shader macros
void ShaderDX11::DeleteMacro(const std::string & key)
{
	m_macros.erase(key);
}

// Shader macros
void ShaderDX11::ClearMacros(void)
{
	m_macros.clear();
}


// Process a debug command that was propogated from the render asset manager
bool ShaderDX11::ProcessCommand(GameConsoleCommand & command)
{
	// Asset commands: InputCmd == "asset", Param(0) == <asset-class>", Param(1) == [ <asset-name> | [ * | "all" ] ], Param(2+) == <actual-commands>
	if (command.Parameter(2) == "reload")
	{
		Game::Log << LOG_INFO << "Reloading shader bytecode for \"" << m_name << "\"\n";
		Reload();
		Game::Engine->GetRenderDevice()->ReinitialiseRenderProcessesForShaderReload();
		command.SetSuccessOutput(concat("Reloaded shader \"")(m_name)("\"").str());
		return true;
	}
	else if (command.Parameter(2) == "macro")
	{
		if (command.Parameter(3) == "set" && command.ParameterCount() >= 6)
		{
			SetMacro(command.Parameter(4), command.Parameter(5));
			command.SetSuccessOutput(concat("Shader \"")(m_name)("\" macro \"")(command.Parameter(4))("\" set to \"")(command.Parameter(5))("\"").str());
		}
		else if (command.Parameter(3) == "delete" && command.ParameterCount() >= 5)
		{
			DeleteMacro(command.Parameter(4));
			command.SetSuccessOutput(concat("Shader \"")(m_name)("\" macro \"")(command.Parameter(4))("\" was deleted").str());
		}
		else if (command.Parameter(3) == "purge")
		{
			ClearMacros();
			command.SetSuccessOutput(concat("Shader \"")(m_name)("\": all macro data purged").str());
		}
		else if (command.Parameter(3) == "list")
		{
			command.SetSuccessOutput(concat("Shader \"")(m_name)("\" macro set: { ")(GetShaderMacroString())(" }").str());
		}
		else
		{
			command.SetFailureOutput(concat("Unknown shader macro command \"")(command.Parameter(3))("\"").str());
		}
		return true;
	}

	return false;
}



// Determine DXGI format
// Info from: http://takinginitiative.net/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/
DXGI_FORMAT GetDXGIFormat(const D3D11_SIGNATURE_PARAMETER_DESC& paramDesc)
{
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	if (paramDesc.Mask == 1) // 1 component
	{
		switch (paramDesc.ComponentType)
		{
		case D3D_REGISTER_COMPONENT_UINT32:
		{
			format = DXGI_FORMAT_R32_UINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_SINT32:
		{
			format = DXGI_FORMAT_R32_SINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
		{
			format = DXGI_FORMAT_R32_FLOAT;
		}
		break;
		}
	}
	else if (paramDesc.Mask <= 3) // 2 components
	{
		switch (paramDesc.ComponentType)
		{
		case D3D_REGISTER_COMPONENT_UINT32:
		{
			format = DXGI_FORMAT_R32G32_UINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_SINT32:
		{
			format = DXGI_FORMAT_R32G32_SINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
		{
			format = DXGI_FORMAT_R32G32_FLOAT;
		}
		break;
		}
	}
	else if (paramDesc.Mask <= 7) // 3 components
	{
		switch (paramDesc.ComponentType)
		{
		case D3D_REGISTER_COMPONENT_UINT32:
		{
			format = DXGI_FORMAT_R32G32B32_UINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_SINT32:
		{
			format = DXGI_FORMAT_R32G32B32_SINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
		{
			format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		break;
		}
	}
	else if (paramDesc.Mask <= 15) // 4 components
	{
		switch (paramDesc.ComponentType)
		{
		case D3D_REGISTER_COMPONENT_UINT32:
		{
			format = DXGI_FORMAT_R32G32B32A32_UINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_SINT32:
		{
			format = DXGI_FORMAT_R32G32B32A32_SINT;
		}
		break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
		{
			format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		break;
		}
	}

	return format;
}

std::string ShaderDX11::GetLatestProfile(Shader::Type type) const
{
	// Query the current feature level:
	D3D_FEATURE_LEVEL featureLevel = Game::Engine->GetDevice()->GetFeatureLevel();

	switch (type)
	{
	case Shader::Type::VertexShader:
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "vs_5_0";
			break;
		case D3D_FEATURE_LEVEL_10_1:
			return "vs_4_1";
			break;
		case D3D_FEATURE_LEVEL_10_0:
			return "vs_4_0";
			break;
		case D3D_FEATURE_LEVEL_9_3:
			return "vs_4_0_level_9_3";
			break;
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_1:
			return "vs_4_0_level_9_1";
			break;
		}
		break;
	case Shader::Type::DomainShader:
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "ds_5_0";
			break;
		}
		break;
	case Shader::Type::HullShader:
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "hs_5_0";
			break;
		}
		break;
	case Shader::Type::GeometryShader:
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "gs_5_0";
			break;
		case D3D_FEATURE_LEVEL_10_1:
			return "gs_4_1";
			break;
		case D3D_FEATURE_LEVEL_10_0:
			return "gs_4_0";
			break;
		}
		break;
	case Shader::Type::PixelShader:
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "ps_5_0";
			break;
		case D3D_FEATURE_LEVEL_10_1:
			return "ps_4_1";
			break;
		case D3D_FEATURE_LEVEL_10_0:
			return "ps_4_0";
			break;
		case D3D_FEATURE_LEVEL_9_3:
			return "ps_4_0_level_9_3";
			break;
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_1:
			return "ps_4_0_level_9_1";
			break;
		}
		break;
	case Shader::Type::ComputeShader:
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			return "cs_5_0";
			break;
		case D3D_FEATURE_LEVEL_10_1:
			return "cs_4_1";
			break;
		case D3D_FEATURE_LEVEL_10_0:
			return "cs_4_0";
			break;
		}
	} // switch( type )

	Game::Log << LOG_ERROR << "Could not establish supported shader feature levels; unknown shader type code " << (int)type << "\n";
	return "";
}