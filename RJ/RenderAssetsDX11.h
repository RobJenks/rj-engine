#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "ManagedPtr.h"
#include "Shader.h"
#include "Texture.h"
#include "InputLayoutDesc.h"
#include "CPUGraphicsResourceAccess.h"
#include "Data\Shaders\Common\CommonShaderConstantBufferDefinitions.hlsl.h"
#include "ConstantBufferDX11.h";
#include "VertexBufferDX11.h"
class ShaderDX11;
class SamplerStateDX11;
class RenderTargetDX11;
class MaterialDX11;
class PipelineStateDX11;
class TextureDX11;
class VertexBufferDX11;

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


	RenderAssetsDX11(void);


	CMPINLINE const ShaderCollection &				GetShaders(void) const { return m_shaders; }
	CMPINLINE const SamplerStateCollection &		GetSamplerStates(void) const { return m_samplers; }
	CMPINLINE const RenderTargetCollection &		GetRenderTargets(void) const { return m_rendertargets; }
	CMPINLINE const MaterialCollection &			GetMaterials(void) const { return m_materials; }
	CMPINLINE const PipelineStateCollection &		GetPipelineStates(void) const { return m_pipelinestates; }
	CMPINLINE const TextureCollection &				GetTextures(void) const { return m_textures; }
	CMPINLINE const ConstantBufferCollection &		GetConstantBuffers(void) const { return m_constantbuffers; }
	CMPINLINE const VertexBufferCollection &		GetVertexBuffers(void) const { return m_vertexbuffers; }

	CMPINLINE ShaderDX11 *							GetShader(const std::string & name) { return GetAsset<ShaderDX11>(name, m_shaders); }
	CMPINLINE SamplerStateDX11 *					GetSamplerState(const std::string & name) { return GetAsset<SamplerStateDX11>(name, m_samplers); }
	CMPINLINE RenderTargetDX11 *					GetRenderTarget(const std::string & name) { return GetAsset<RenderTargetDX11>(name, m_rendertargets); }
	CMPINLINE MaterialDX11 *						GetMaterial(const std::string & name) { return GetAsset<MaterialDX11>(name, m_materials); }
	CMPINLINE PipelineStateDX11 *					GetPipelineState(const std::string & name) { return GetAsset<PipelineStateDX11>(name, m_pipelinestates); }
	CMPINLINE TextureDX11 *							GetTexture(const std::string & name) { return GetAsset<TextureDX11>(name, m_textures); }
	CMPINLINE ConstantBufferDX11 *					GetConstantBuffer(const std::string & name) { return GetAsset<ConstantBufferDX11>(name, m_constantbuffers); }
	CMPINLINE VertexBufferDX11 *					GetVertexBuffer(const std::string & name) { return GetAsset<VertexBufferDX11>(name, m_vertexbuffers); }

public:

	CMPINLINE SamplerStateDX11 *					CreateSamplerState(const std::string & name) { return CreateAsset<SamplerStateDX11>(name, m_samplers); }
	CMPINLINE RenderTargetDX11 *					CreateRenderTarget(const std::string & name) { return CreateAsset<RenderTargetDX11>(name, m_rendertargets); }
	CMPINLINE MaterialDX11 *						CreateMaterial(const std::string & name) { return CreateAsset<MaterialDX11>(name, m_materials); }
	CMPINLINE PipelineStateDX11 *					CreatePipelineState(const std::string & name) { return CreateAsset<PipelineStateDX11>(name, m_pipelinestates); }

	template <typename T>
	ConstantBufferDX11 *							CreateConstantBuffer(const std::string & name);
	template <typename T>
	ConstantBufferDX11 *							CreateConstantBuffer(const std::string & name, const T *data);

	template <typename TVertex>
	VertexBufferDX11 *								CreateVertexBuffer(const std::string & name, const TVertex *data, UINT count, UINT stride);
	template <typename TVertex>
	VertexBufferDX11 *								CreateVertexBuffer(const std::string & name, UINT count);


	TextureDX11 *									CreateTexture(const std::string & name);
	TextureDX11 *									CreateTexture1D(const std::string & name, uint16_t width, uint16_t slices = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTexture2D(const std::string & name, uint16_t width, uint16_t height, uint16_t slices = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTexture3D(const std::string & name, uint16_t width, uint16_t height, uint16_t depth, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTextureCube(const std::string & name, uint16_t size, uint16_t numCubes = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);

	Result											InitialiseExternalShaderResource(ShaderDX11 ** ppOutShader, Shader::Type shadertype, const std::string & fileName,
															const std::string & entryPoint, const std::string & profile, const InputLayoutDesc *input_layout = NULL);
	

private:

	TextureDX11 *									RegisterNewTexture(const std::string & name, std::unique_ptr<TextureDX11> texture);

	template <class T>
	T *												CreateAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData);

	template <class T>
	T *												GetAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData);



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
T *	RenderAssetsDX11::GetAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData)
{
	auto it = assetData.find(name);
	return (it != assetData.end() ? it->second.get() : NULL);
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
VertexBufferDX11 * RenderAssetsDX11::CreateVertexBuffer(const std::string & name, UINT count)
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

	VertexBufferDX11 *buffer = CreateVertexBuffer<TVertex>(name, data, count, sizeof(TVertex));

	// Deallocate temporary data before returning
	SafeDeleteArray(data);

	return buffer;
}

template <typename TVertex>
VertexBufferDX11 * RenderAssetsDX11::CreateVertexBuffer(const std::string & name, const TVertex *data, UINT count, UINT stride)
{
	if (name.empty()) { Game::Log << LOG_ERROR << "Cannot create vertex buffer with null identifier\n"; return NULL; }
	if (!data) { Game::Log << LOG_ERROR << "Cannot create vertex buffer \"" << name << "\" with null data reference\n"; return NULL; }
	if (count == 0U || stride == 0U) { Game::Log << LOG_ERROR << "Cannot create vertex buffer \"" << name << "\"; invalid size data (vc=" << count << ", vs=" << stride << ")\n"; return NULL; }

	if (m_vertexbuffers.find(name) != m_vertexbuffers.end())
	{
		Game::Log << LOG_ERROR << "Cannot create vertex buffer \"" << name << "\"; asset already exists with this identifier\n";
		return NULL;
	}

	m_vertexbuffers[name] = std::make_unique<VertexBufferDX11>((const void*)data, count, stride);
	return m_vertexbuffers[name].get();
}



template <class T>
T *	RenderAssetsDX11::CreateAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData)
{

	if (name.empty())
	{
		constexpr std::string type = STRING(T);
		constexpr size_t ix = type.find_last_of("DX11");
		constexpr if (ix != std::string::npos) type = type.substr(0U, ix);

		Game::Log << LOG_ERROR << "Cannot initialise " << type << " definition with null identifier\n"; return NULL;
	}

	if (assetData.find(name) != assetData.end())
	{
		constexpr std::string type = STRING(T);
		constexpr size_t ix = type.find_last_of("DX11");
		constexpr if (ix != std::string::npos) type = type.substr(0U, ix);

		Game::Log << LOG_WARN << type << " definition for \"" << name << "\" already exists, cannot create duplicate\n";
		return NULL;
	}

	assetData[name] = std::make_unique<T>();
	return assetData[name].get();
}



