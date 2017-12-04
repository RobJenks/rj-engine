#include <d3dcompiler.h>
#include <filesystem>
#include <limits>
#include "ShaderDX11.h"
#include "Utility.h"
#include "Logging.h"
#include "DX11_Core.h"
#include "CoreEngine.h"

// TODO: VS2017 is still implementing as exp branch; convert to std library once available
namespace fs = std::experimental::filesystem;

// Initialise static data
ShaderDX11::ShaderParameterSet::size_type ShaderDX11::INVALID_SHADER_PARAMETER = (std::numeric_limits<ShaderParameterSet::size_type>::max)();


ShaderDX11::ShaderDX11(void)
	: 
	m_type(Shader::Type::SHADER_TYPE_COUNT), 
	m_vs(NULL), m_ps(NULL), m_gs(NULL), m_hs(NULL), m_ds(NULL), m_cs(NULL), 
	m_inputlayout(NULL), 
	m_shaderblob(NULL), 
	m_slot_material(Shader::NO_SLOT_ID)
{
}

ShaderDX11::~ShaderDX11()
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

	Game::Log << LOG_ERROR << "Could not establish supported shader feature levels; unknown shader type code " << type << "\n";
	return "";
}

bool ShaderDX11::LoadShaderFromString(	Shader::Type shadertype, const std::string& shaderSource, const std::wstring& sourceFileName, 
										const std::string& entryPoint, const std::string& profile, const InputLayoutDesc *input_layout = NULL)
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
				Game::Log << LOG_ERROR << "Invalid shader type " << shadertype << " for \"" << entryPoint << "\", could not determine supported feature level (" << sourceFileName << ")\n";
				return false;
			}
		}

		// Get global compiled macro set
		ShaderMacros::CompiledMacroData macros = ShaderManager::GetGlobalShaderMacros().GetCompiledData();

		// Compilation flags
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if		defined( _DEBUG )
			flags |= D3DCOMPILE_DEBUG;
#		endif

		// Attempt to load and compile the HLSL
		fs::path filePath(sourceFileName);
		std::string sourceFilePath = filePath.string();

		hr = D3DCompile((LPCVOID)shaderSource.c_str(), shaderSource.size(), sourceFilePath.c_str(), macros.data(), 
			D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), _profile.c_str(), flags, 0, &shaderBlob, &errorBlob);

		// Check compilation results and report any errors
		if (FAILED(hr))
		{
			Game::Log << LOG_ERROR << "Compilation of shader \"" << entryPoint << "\" failed (hr=" << hr << ", file=" << sourceFileName << ")\n";
			if (errorBlob)
			{
				Game::Log << LOG_ERROR << "Error buffer ptr: " << (static_cast<char*>(errorBlob->GetBufferPointer()));
			}
			else
			{
				Game::Log << LOG_ERROR << "No further error details available; no error buffer generated";
			}
			return false;
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
			Game::Log << LOG_ERROR << "Invalid shader type " << shadertype << " for " << entryPoint << "; cannnot create compiled shader object (" << sourceFileName << ")\n";
			return false;
	}

	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Failed to create shader \"" << entryPoint << "\" from compiled source (hr=" << hr << ", file=" << sourceFileName << ")\n";
		return false;
	}

	// Compile the shader input layout, if applicable
	if (input_layout)
	{
		hr = device->CreateInputLayout(input_layout->Data(), input_layout->ElementCount(), m_shaderblob->GetBufferPointer(), m_shaderblob->GetBufferSize(), &m_inputlayout);
		if (FAILED(hr))
		{
			Game::Log << LOG_ERROR << "Failed to create input layout for \"" << entryPoint << "\" (hr=" << hr << ", file=" << sourceFileName << ")\n";
			return false;
		}
	}

	// We have successfully loaded and compiled the shader
	return true;
}

bool ShaderDX11::LoadShaderFromFile(Shader::Type shadertype, const std::wstring& fileName, const std::string& entryPoint, 
									const std::string& profile, const InputLayoutDesc *input_layout = NULL)
{
	bool result = false;

	fs::path filePath(fileName);
	if (fs::exists(filePath) && fs::is_regular_file(filePath))
	{
		// Store data necessary to reload the shader if it changes on disc.
		m_filename = fileName;
		m_entrypoint = entryPoint;
		m_profile = profile;

		// Load from disk
		std::ifstream inputFile(fileName);
		std::string source((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

		result = LoadShaderFromString(shadertype, source, fileName, entryPoint, profile, input_layout);
	}
	else
	{
		Game::Log << LOG_ERROR << "Failed to load shader \"" << entryPoint << "\" from external file \"" << fileName << "\"; source file may not exist or be otherwise inacessible\n";
	}

	return result;
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
		Game::Log << LOG_WARN << "Attempted to retrieve shader parameter \"" << name << "\" from \"" << m_entrypoint << "\"; does not exist";
		return INVALID_SHADER_PARAMETER;
	}
}

void ShaderDX11::Bind()
{
	auto devicecontext = Game::Engine->GetDeviceContext();

	// Bind all required shader parameters
	for (auto parameter : m_parameters)
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
	for (auto parameter : m_parameters)
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
void ShaderDX11::Dispatch(const XMFLOAT3 & numGroups)
{
	if (m_cs)
	{
		Game::Engine->GetDeviceContext()->Dispatch(numGroups.x, numGroups.y, numGroups.z);
	}
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