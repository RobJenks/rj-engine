#include "RenderAssetsDX11.h"
#include "Logging.h"
#include "GameDataExtern.h"
#include "TextureDX11.h"
#include "ShaderDX11.h"
#include "GameConsoleCommand.h"


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


// Virtual inherited method to accept a command from the console
bool RenderAssetsDX11::ProcessConsoleCommand(GameConsoleCommand & command)
{
	// Asset commands: InputCmd == "asset", Param(0) == <asset-class>", Param(1) == [ <asset-name> | [ * | "all" ] ]
	if (command.InputCommand == "asset" && command.ParameterCount() >= 2 &&				
		command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)
	{
		std::string type = command.Parameter(0);	
		std::string name = command.Parameter(1);
		StrLowerC(type);

		// Broadcast to entire asset classes, or specific items within the class
		if (name == "*" || name == "all")
		{
			if (type == "shader")		BROADCAST_DEBUG_COMMAND_TO_ASSET_CLASS(ShaderDX11, command)
			/// elseif ...others
		}
		else
		{
			if (type == "shader")		PROPOGATE_DEBUG_COMMAND_TO_ASSET(ShaderDX11, name, command)
			/// elseif ...others
		}
		

		/* Verify whether the target assets did process this command */
		return (command.OutputStatus != GameConsoleCommand::CommandResult::NotExecuted);
	}

	return false;
}

// Destructor
RenderAssetsDX11::~RenderAssetsDX11(void)
{
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


Result RenderAssetsDX11::InitialiseExternalShaderResource(	ShaderDX11 ** ppOutShader, const std::string & name, Shader::Type shadertype, const std::string & fileName, 
															const std::string & entryPoint, const std::string & profile, const InputLayoutDesc *input_layout, 
															const ShaderMacros::MacroData & macros)
{
	Game::Log << LOG_INFO << "Initialising shader \"" << name << "\" from \"" << fileName << "\"\n";

	// No duplicates allowed
	if (AssetExists<ShaderDX11>(name))
	{
		Game::Log << LOG_ERROR << "Multiple shader resources detected with name \"" << name << "\"\n";
		return ErrorCodes::CannotLoadDuplicateShaderResource;
	}

	// Verify shader pointer provided for initialisation
	if (ppOutShader && (*ppOutShader != NULL))
	{
		Game::Log << LOG_WARN << "Shader resource already exists, existing resource will be deallocated and overwritten\n";
		SafeDelete(*ppOutShader);
	}

	// Attempt to initialise from file
	ShaderDX11 *shader = CreateAsset<ShaderDX11>(name);
	bool success = shader->LoadShaderFromFile(name, shadertype, ConvertStringToWString(BuildStrFilename(D::DATA, fileName)), entryPoint, profile, input_layout, macros);

	// Deallocate the shader object if initialisation failed
	if (!success)
	{
		Game::Log << LOG_ERROR << "Initialisation of shader \"" << name << "\" failed, cannot proceed\n";
		SafeDelete(shader);
		return ErrorCodes::CannotLoadShaderFromFile;
	}

	// Return a pointer to the shader if required
	if (ppOutShader) (*ppOutShader) = shader;

	Game::Log << LOG_INFO << "Shader \"" << name << "\" loaded successfully\n";
	return ErrorCodes::NoError;
}


