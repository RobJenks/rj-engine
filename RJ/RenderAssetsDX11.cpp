#include "RenderAssetsDX11.h"
#include "Logging.h"
#include "GameDataExtern.h"
#include "TextureDX11.h"
#include "ShaderDX11.h"


// Initialise static data
std::string RenderAssetsDX11::DEFAULT_ASSET_ID = "DEFAULT";


// Constructor
RenderAssetsDX11::RenderAssetsDX11(void)
{

}

// Primary initialisation method; called by the render device that owns this asset data
void RenderAssetsDX11::Initialise(void)
{
	Game::Log << LOG_INFO << "Initialising render device asset data store\n";

	InitialiseDefaultAssets();

	Game::Log << LOG_INFO << "Render device asset data store initialised\n";
}

void RenderAssetsDX11::InitialiseDefaultAssets(void)
{
	// Default material; uses all parameters set in default constructor so no initialisation required
	CreateMaterial(RenderAssetsDX11::DEFAULT_ASSET_ID);

}







TextureDX11 * RenderAssetsDX11::CreateTexture(const std::string & name)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>()));
}

TextureDX11 * RenderAssetsDX11::CreateTexture1D(const std::string & name, uint16_t width, uint16_t slices, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(width, slices, format, cpuAccess, gpuWrite)));
}

TextureDX11 * RenderAssetsDX11::CreateTexture2D(const std::string & name, uint16_t width, uint16_t height, uint16_t slices, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(width, height, slices, format, cpuAccess, gpuWrite)));
}

TextureDX11 * RenderAssetsDX11::CreateTexture3D(const std::string & name, uint16_t width, uint16_t height, uint16_t depth, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(TextureDX11::Tex3d, width, height, depth, format, cpuAccess, gpuWrite)));
}

TextureDX11 * RenderAssetsDX11::CreateTextureCube(const std::string & name, uint16_t size, uint16_t numCubes, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(TextureDX11::Cube, size, numCubes, format, cpuAccess, gpuWrite)));
}

// Attempts to register the given texture.  Returns a pointer to the underlying resource if successful.  Will return NULL, 
// deallocate any underlying resource and report an error if the registration fails
TextureDX11 * RenderAssetsDX11::RegisterNewTexture(const std::string & name, std::unique_ptr<TextureDX11> texture)
{
	if (name.empty())
	{
		Game::Log << LOG_ERROR << "Cannot register texture resource with null identifier\n";
		return  NULL;
	}

	if (texture.get() == NULL)
	{
		Game::Log << LOG_ERROR << "Cannot register \"" << name << "\" as null texture resource\n";
		return NULL;
	}

	if (m_textures.find(name) != m_textures.end())
	{
		Game::Log << LOG_WARN << "Cannot register texxture \"" << name << "\"; resource already exists with this identifier\n";
		return NULL;
	}

	// Store the asset identifier within this texture as well
	texture.get()->SetCode(name);

	// Store the new asset and return a pointer
	m_textures[name] = std::move(texture);
	Game::Log << LOG_INFO << "Registered new texture resource \"" << name << "\"\n";
	return m_textures[name].get();
}


Result RenderAssetsDX11::InitialiseExternalShaderResource(	ShaderDX11 ** ppOutShader, Shader::Type shadertype, const std::string & fileName, const std::string & entryPoint,
															const std::string & profile, const InputLayoutDesc *input_layout)
{
	Game::Log << LOG_INFO << "Initialising shader \"" << entryPoint << "\" from \"" << fileName << "\"\n";

	// No duplicates allowed
	if (AssetExists<ShaderDX11>(entryPoint))
	{
		Game::Log << LOG_ERROR << "Multiple shader resources detected with entry point \"" << entryPoint << "\"\n";
		return ErrorCodes::CannotLoadDuplicateShaderResource;
	}

	// Verify shader pointer provided for initialisation
	if (!ppOutShader) return ErrorCodes::InvalidShaderReferenceProvidedForInitialisation;
	if (*ppOutShader)
	{
		Game::Log << LOG_WARN << "Shader resource already exists, existing resource will be deallocated and overwritten\n";
		SafeDelete(*ppOutShader);
	}

	// Attempt to initialise from file
	(*ppOutShader) = CreateAsset<ShaderDX11>(entryPoint);
	bool success = (*ppOutShader)->LoadShaderFromFile(shadertype, ConvertStringToWString(BuildStrFilename(D::DATA, fileName)), entryPoint, profile, input_layout);

	// Deallocate the shader object if initialisation failed
	if (!success)
	{
		Game::Log << LOG_ERROR << "Initialisation of shader \"" << entryPoint << "\" failed, cannot proceed\n";
		SafeDelete(*ppOutShader);
		return ErrorCodes::CannotLoadShaderFromFile;
	}

	Game::Log << LOG_INFO << "Shader \"" << entryPoint << "\" loaded successfully\n";
	return ErrorCodes::NoError;
}


