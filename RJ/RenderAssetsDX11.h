#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Logging.h"
#include "ManagedPtr.h"
#include "Shader.h"
#include "InputLayoutDesc.h"
#include "CPUGraphicsResourceAccess.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "ConstantBufferDX11.h"
#include "VertexBufferDX11.h"
#include "DynamicVertexBufferDX11.h"
#include "ShaderDX11.h"
#include "SamplerStateDX11.h"
#include "RenderTargetDX11.h"
#include "MaterialDX11.h"
#include "PipelineStateDX11.h"
#include "TextureDX11.h"

// Compiler flag which enables more verbose reporting of asset errors in debug mode
#define PERFORM_ASSET_ERROR_LOGGING

class RenderAssetsDX11
{
public:

	typedef std::unordered_map<std::string, std::unique_ptr<ShaderDX11>> ShaderCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<SamplerStateDX11>> SamplerStateCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<RenderTargetDX11>> RenderTargetCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<MaterialDX11>> MaterialCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<PipelineStateDX11>> PipelineStateCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<TextureDX11>> TextureCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<ConstantBufferDX11>> ConstantBufferCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<VertexBufferDX11>> VertexBufferCollection;

	static std::string								DEFAULT_ASSET_ID;

	RenderAssetsDX11(void);

	// Primary initialisation method; called by the render device that owns this asset data
	void											Initialise(void);

	void											InitialiseDefaultAssets(void);


	CMPINLINE const ShaderCollection &				GetShaders(void) const { return m_shaders; }
	CMPINLINE const SamplerStateCollection &		GetSamplerStates(void) const { return m_samplers; }
	CMPINLINE const RenderTargetCollection &		GetRenderTargets(void) const { return m_rendertargets; }
	CMPINLINE const MaterialCollection &			GetMaterials(void) const { return m_materials; }
	CMPINLINE const PipelineStateCollection &		GetPipelineStates(void) const { return m_pipelinestates; }
	CMPINLINE const TextureCollection &				GetTextures(void) const { return m_textures; }
	CMPINLINE const ConstantBufferCollection &		GetConstantBuffers(void) const { return m_constantbuffers; }
	CMPINLINE const VertexBufferCollection &		GetVertexBuffers(void) const { return m_vertexbuffers; }

	CMPINLINE ShaderDX11 *							GetShader(const std::string & name) { return GetAsset<ShaderDX11>(name); }
	CMPINLINE SamplerStateDX11 *					GetSamplerState(const std::string & name) { return GetAsset<SamplerStateDX11>(name); }
	CMPINLINE RenderTargetDX11 *					GetRenderTarget(const std::string & name) { return GetAsset<RenderTargetDX11>(name); }
	CMPINLINE MaterialDX11 *						GetMaterial(const std::string & name) { return GetAsset<MaterialDX11>(name); }
	CMPINLINE PipelineStateDX11 *					GetPipelineState(const std::string & name) { return GetAsset<PipelineStateDX11>(name); }
	CMPINLINE TextureDX11 *							GetTexture(const std::string & name) { return GetAsset<TextureDX11>(name); }
	CMPINLINE ConstantBufferDX11 *					GetConstantBuffer(const std::string & name) { return GetAsset<ConstantBufferDX11>(name); }
	CMPINLINE VertexBufferDX11 *					GetVertexBuffer(const std::string & name) { return GetAsset<VertexBufferDX11>(name); }

	template <typename T>
	bool											AssetExists(std::string name);
	
	template <typename T>
	T *												GetOrCreateAsset(const std::string & name);

	CMPINLINE MaterialDX11 *						GetDefaultMaterial(void) { return GetDefaultAsset<MaterialDX11>(); }

public:

	CMPINLINE SamplerStateDX11 *					CreateSamplerState(const std::string & name) { return CreateAsset<SamplerStateDX11>(name); }
	CMPINLINE RenderTargetDX11 *					CreateRenderTarget(const std::string & name) { return CreateAsset<RenderTargetDX11>(name); }
	CMPINLINE MaterialDX11 *						CreateMaterial(const std::string & name) { return CreateAsset<MaterialDX11>(name); }
	CMPINLINE PipelineStateDX11 *					CreatePipelineState(const std::string & name) { return CreateAsset<PipelineStateDX11>(name); }

	template <typename T>
	ConstantBufferDX11 *							CreateConstantBuffer(const std::string & name);
	template <typename T>
	ConstantBufferDX11 *							CreateConstantBuffer(const std::string & name, const T *data);

	template <typename TVertex>
	VertexBufferDX11 *								CreateVertexBuffer(const std::string & name, const TVertex *data, UINT count, UINT stride, bool dynamic_vb = false);
	template <typename TVertex>
	VertexBufferDX11 *								CreateVertexBuffer(const std::string & name, UINT count, bool dynamic_vb = false);


	TextureDX11 *									CreateTexture(const std::string & name);
	TextureDX11 *									CreateTexture1D(const std::string & name, uint16_t width, uint16_t slices = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTexture2D(const std::string & name, uint16_t width, uint16_t height, uint16_t slices = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTexture3D(const std::string & name, uint16_t width, uint16_t height, uint16_t depth, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTextureCube(const std::string & name, uint16_t size, uint16_t numCubes = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);

	Result											InitialiseExternalShaderResource(ShaderDX11 ** ppOutShader, Shader::Type shadertype, const std::string & fileName,
															const std::string & entryPoint, const std::string & profile, const InputLayoutDesc *input_layout = NULL);
	
public:

	template <typename T>
	void											DeleteAsset(const std::string & name);

	CMPINLINE void									DeleteTexture(const std::string & name) { DeleteAsset<TextureDX11>(name); }


private:

	TextureDX11 *									RegisterNewTexture(const std::string & name, std::unique_ptr<TextureDX11> texture);

	template <class T>
	T *												CreateAsset(const std::string & name);

	template <class T>
	T *												GetAsset(const std::string & name);

	template <class T>
	T *												GetDefaultAsset(void);

private:

	template <typename T> CMPINLINE constexpr std::unordered_map<std::string, std::unique_ptr<T>> & 	GetAssetData(void) { throw("Invalid asset data type"); }
	template <> CMPINLINE constexpr ShaderCollection & 			GetAssetData<ShaderDX11>(void) { return m_shaders; }
	template <> CMPINLINE constexpr SamplerStateCollection & 	GetAssetData<SamplerStateDX11>(void) { return m_samplers; }
	template <> CMPINLINE constexpr RenderTargetCollection & 	GetAssetData<RenderTargetDX11>(void) { return m_rendertargets; }
	template <> CMPINLINE constexpr MaterialCollection & 		GetAssetData<MaterialDX11>(void) { return m_materials; }
	template <> CMPINLINE constexpr PipelineStateCollection & 	GetAssetData<PipelineStateDX11>(void) { return m_pipelinestates; }
	template <> CMPINLINE constexpr TextureCollection & 		GetAssetData<TextureDX11>(void) { return m_textures; }
	template <> CMPINLINE constexpr ConstantBufferCollection & 	GetAssetData<ConstantBufferDX11>(void) { return m_constantbuffers; }
	template <> CMPINLINE constexpr VertexBufferCollection & 	GetAssetData<VertexBufferDX11>(void) { return m_vertexbuffers; }

private:

	template <typename T> CMPINLINE constexpr const char *		AssetName(void) const { return "<unknown-asset-type>"; }
	template <> CMPINLINE constexpr const char *				AssetName<ShaderDX11>(void) const { return "Shader"; }
	template <> CMPINLINE constexpr const char *				AssetName<SamplerStateDX11>(void) const { return "SamplerState"; }
	template <> CMPINLINE constexpr const char *				AssetName<RenderTargetDX11>(void) const { return "RenderTarget"; }
	template <> CMPINLINE constexpr const char *				AssetName<MaterialDX11>(void) const { return "Material"; }
	template <> CMPINLINE constexpr const char *				AssetName<PipelineStateDX11>(void) const { return "PipelineState"; }
	template <> CMPINLINE constexpr const char *				AssetName<TextureDX11>(void) const { return "Texture"; }
	template <> CMPINLINE constexpr const char *				AssetName<ConstantBufferDX11>(void) const { return "ConstantBuffer"; }
	template <> CMPINLINE constexpr const char *				AssetName<VertexBufferDX11>(void) const { return "VertexBuffer"; }


private:

	ShaderCollection								m_shaders;
	SamplerStateCollection							m_samplers;
	RenderTargetCollection							m_rendertargets;
	MaterialCollection								m_materials;
	PipelineStateCollection							m_pipelinestates;
	TextureCollection								m_textures;
	ConstantBufferCollection						m_constantbuffers;
	VertexBufferCollection							m_vertexbuffers;

};



template <class T>
T *	RenderAssetsDX11::GetAsset(const std::string & name)
{
	auto & assetData = GetAssetData<T>();
	auto it = assetData.find(name);

#if defined(_DEBUG) && defined(PERFORM_ASSET_ERROR_LOGGING)

	if (it != assetData.end()) return it->second.get();
	else
	{
		Game::Log << LOG_WARN << "Failed to retrieve " << AssetName<T>() << " asset \"" << name << "\"\n";
		return NULL;
	}

#else
	return (it != assetData.end() ? it->second.get() : NULL);
#endif
	
}

template <class T>
T *	RenderAssetsDX11::GetDefaultAsset(void)
{
	return GetAsset<T>(RenderAssetsDX11::DEFAULT_ASSET_ID);
}

template <typename T>
bool RenderAssetsDX11::AssetExists(std::string name)
{
	auto & data = GetAssetData<T>();
	return (data.find(name) != data.end());
}

template <typename T>
T * RenderAssetsDX11::GetOrCreateAsset(const std::string & name)
{
	auto & data = GetAssetData<T>();
	auto it = data.find(name);

	if (it != data.end())
	{
		return it->second.get();
	}
	else
	{
		return CreateAsset<T>(name);
	}
}


template <typename T>
ConstantBufferDX11 * RenderAssetsDX11::CreateConstantBuffer(const std::string & name)
{
	return CreateConstantBuffer<T>(name, NULL);
}

template <typename T>
ConstantBufferDX11 * RenderAssetsDX11::CreateConstantBuffer(const std::string & name, const T *data)
{
	if (name.empty())
	{
		Game::Log << LOG_ERROR << "Cannot create constant buffer with null identifier\n";
		return NULL;
	}

	if (m_constantbuffers.find(name) != m_constantbuffers.end())
	{
		Game::Log << LOG_ERROR << "Cannot create constant buffer \"" << name << "\"; buffer already exists with this identifier\n";
		return NULL;
	}

	ConstantBufferDX11 *buffer = ConstantBufferDX11::Create(data);
	if (!buffer)
	{
		Game::Log << LOG_ERROR << "Failed to create constant buffer \"" << name << "\" (sz:" << sizeof(T) << ", d" << (data ? "!=0" : "==0") << ")\n";
		return NULL;
	}

	m_constantbuffers[name] = std::unique_ptr<ConstantBufferDX11>(buffer);
	return m_constantbuffers[name].get();
}


template <typename TVertex>
VertexBufferDX11 * RenderAssetsDX11::CreateVertexBuffer(const std::string & name, UINT count, bool dynamic_vb)
{
	if (count == 0U)
	{
		Game::Log << LOG_ERROR << "Cannot create new vertex buffer with vcount == 0\n";
		return NULL;
	}

	// Method does not request a pre-allocated buffer for initialisation.  We therefore perform a temporary allocation here
	TVertex *data = new TVertex[count];
	if (!data)
	{
		Game::Log << LOG_ERROR << "Failed to allocate vertex buffer data on initialisation (vc=" << count << ", vs=" << sizeof(TVertex) << ")\n";
		return NULL;
	}

	VertexBufferDX11 *buffer = CreateVertexBuffer<TVertex>(name, data, count, sizeof(TVertex), dynamic_vb);

	// Deallocate temporary data before returning
	SafeDeleteArray(data);

	return buffer;
}

template <typename TVertex>
VertexBufferDX11 * RenderAssetsDX11::CreateVertexBuffer(const std::string & name, const TVertex *data, UINT count, UINT stride, bool dynamic_vb)
{
	if (name.empty()) { Game::Log << LOG_ERROR << "Cannot create vertex buffer with null identifier\n"; return NULL; }
	if (!data) { Game::Log << LOG_ERROR << "Cannot create vertex buffer \"" << name << "\" with null data reference\n"; return NULL; }
	if (count == 0U || stride == 0U) { Game::Log << LOG_ERROR << "Cannot create vertex buffer \"" << name << "\"; invalid size data (vc=" << count << ", vs=" << stride << ")\n"; return NULL; }

	if (m_vertexbuffers.find(name) != m_vertexbuffers.end())
	{
		Game::Log << LOG_ERROR << "Cannot create vertex buffer \"" << name << "\"; asset already exists with this identifier\n";
		return NULL;
	}

	if (dynamic_vb)
	{
		m_vertexbuffers[name] = std::make_unique<DynamicVertexBufferDX11>((const void*)data, count, stride);
	}
	else
	{
		m_vertexbuffers[name] = std::make_unique<VertexBufferDX11>((const void*)data, count, stride);
	}
	
	
	return m_vertexbuffers[name].get();
}



template <class T>
T *	RenderAssetsDX11::CreateAsset(const std::string & name)
{ 
	auto & assetData = GetAssetData<T>();

	if (name.empty())
	{
		Game::Log << LOG_ERROR << "Cannot initialise " << AssetName<T>() << " definition with null identifier\n"; 	
		return NULL;
	}

	if (assetData.find(name) != assetData.end())
	{
		Game::Log << LOG_WARN << AssetName<T>() << " definition for \"" << name << "\" already exists, cannot create duplicate\n";
		return NULL;
	}

	assetData[name] = std::make_unique<T>();

	T *asset = assetData[name].get();
	__if_exists(T::SetCode)		// TODO: MSVC-specific, can use SFINAE if needed to make more generic
	{
		asset->SetCode(name);
	}

	return asset;
}

template <typename T>
void RenderAssetsDX11::DeleteAsset(const std::string & name)
{
	auto & assetData = GetAssetData<T>();

	if (name.empty())
	{
		Game::Log << LOG_ERROR << "Cannot delete " << AssetName<T>() << " definition with null identifier\n";
		return;
	}

	std::unordered_map<std::string, std::unique_ptr<T>>::iterator it = assetData.find(name);
	if (it != assetData.end())
	{
		assetData.erase(it); 
		Game::Log << LOG_INFO << "Deleted " << AssetName<T>() << " asset \"" << name << "\"\n";
		return;
	}
	else
	{
		Game::Log << LOG_WARN << "Cannot delete " << AssetName<T>() << " asset; no asset exists with name \"" << name << "\"\n";
		return;
	}
}

