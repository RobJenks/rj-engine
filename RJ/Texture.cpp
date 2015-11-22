////////////////////////////////////////////////////////////////////////////////
// Filename: texture.cpp
////////////////////////////////////////////////////////////////////////////////

#include "DX11_Core.h"
#include "DirectXTK\Inc\DDSTextureLoader.h"

#include "ErrorCodes.h"
#include "Utility.h"
#include "GameVarsExtern.h"
#include "CoreEngine.h"

#include "Texture.h"

// Static texture resource collection
std::tr1::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::TextureResources;

// Default constructor
Texture::Texture()
{
	m_texture = NULL;
	m_size = NULL_INTVECTOR2;
	m_filename = "";
}

// Texture passing the filename as a parameter, for convenience
Texture::Texture(const std::string & filename)
{
	// Initialise to default values, in case the load fails
	m_texture = NULL;
	m_size = INTVECTOR2(0, 0);
	m_filename = "";

	// Attempt to load this texture resource
	if (this->Initialise(filename) != ErrorCodes::NoError)
	{
		// The load failed, so set all texture parameters back to defaults
		m_texture = NULL;
		m_size = NULL_INTVECTOR2;
		m_filename = "";
	}
}

Texture::Texture(const Texture& other) { }
Texture::~Texture() { }

// Loads a texture from existing shader resource & description
Result Texture::Initialise(ID3D11Resource *resource, D3D11_SHADER_RESOURCE_VIEW_DESC *resourcedesc)
{
	HRESULT result;

	// Create the texture from supplied resource and SRV desc objects
	result = Game::Engine->GetDevice()->CreateShaderResourceView(resource, resourcedesc, &m_texture);
	if (FAILED(result))
	{
		return ErrorCodes::CouldNotCreateTextureFromD3DResource;
	}
	
	// Also store the filename and size of this texture for future reference
	m_size = Texture::DetermineTextureSize(m_texture);
	m_filename = "";										// No filename since cloned from a resource directly

	// Return success
	return ErrorCodes::NoError;
}

// Loads a texture from file
Result Texture::Initialise(const std::string & filename)
{
	HRESULT result;

	// Check whether this texture has already been loaded (based on its filename)
	ID3D11ShaderResourceView *tex = Texture::Get(filename);
	if (tex)
	{
		// If we have already loaded this texture then simply use a reference to the texture that already exists
		m_texture = tex;
	}
	else
	{
		// Load the texture in from an external file
		result = CreateDDSTextureFromFile(Game::Engine->GetDevice(), ConvertStringToWString(filename).c_str(), NULL, &m_texture);
		if(FAILED(result) || !m_texture)
		{
			return ErrorCodes::CouldNotCreateShaderFromTextureFile;
		}

		// Store a reference to this texture in the global collection for use in future
		Texture::Store(filename, m_texture);
	}

	// Also store the filename and size of this texture for future reference
	m_size = Texture::DetermineTextureSize(m_texture);
	m_filename = filename;

	// Return success
	return ErrorCodes::NoError;
}

// Creates a shader resource view object directly, rather than initialising a texture object
ID3D11ShaderResourceView *Texture::CreateSRV(const string & filename)
{
	// Check whether we have already loaded this resource
	ID3D11ShaderResourceView *res = Texture::Get(filename);
	if (res)
	{
		// If we have already loaded this resource then simply return a reference
		return res;
	}
	else
	{
		// Otherwise, attempt to create a new resource from the specified file
		HRESULT result = CreateDDSTextureFromFile(Game::Engine->GetDevice(), (wchar_t*)filename.c_str(), NULL, &res);
		if(FAILED(result) || !res)
		{
			return NULL;
		}

		// Store a reference to this texture in the global collection for use in future
		Texture::Store(filename, res);

		// Return a reference to the new resource
		return res;
	}
}

Texture *Texture::Clone(void) const
{
	ID3D11Resource *tres;
	D3D11_SHADER_RESOURCE_VIEW_DESC tdesc;
	ID3D11Texture2D *newtex;
	D3D11_TEXTURE2D_DESC texdesc;

	// If this texture object does not contain valid resource information then return a copied null texture
	if (!m_texture)
	{
		return new Texture();
	}

	// Extract resource information from the source texture, for use in creating the new one
	m_texture->GetResource(&tres);
	m_texture->GetDesc(&tdesc);
	
	// Retrieve the texture 2D description for this resource
	ID3D11Texture2D *tex = (ID3D11Texture2D*)tres;
	tex->GetDesc( &texdesc );

	// Use the texture resource info to create a new texture resource
	HRESULT hresult = Game::Engine->GetDevice()->CreateTexture2D(&texdesc, NULL, &newtex);
	if (FAILED(hresult)) return NULL;

	// Copy the resource data into this new Texture2D interface
	Game::Engine->GetDeviceContext()->CopyResource(newtex, tres);

	// Create a new texture object using this new resource
	Texture *texture = new Texture();
	texture->Initialise(newtex, &tdesc);

	// Return a pointer to the new texture
	return texture;
}

INTVECTOR2 Texture::DetermineTextureSize(ID3D11ShaderResourceView *texture)
{
	ID3D11Resource *resource;
	D3D11_TEXTURE2D_DESC desc;
	
	// Make sure have been passed a valid texture resource
	if (!texture) return INTVECTOR2(0, 0);

	// Retrieve details on this resource object
	texture->GetResource( &resource );
	if (!resource) return INTVECTOR2(0, 0);
	((ID3D11Texture2D*)resource)->GetDesc( &desc );
	resource->Release();

	// Now return the size of this texture as an int-vector
	return INTVECTOR2((int)desc.Width, (int)desc.Height);
}

Texture::APPLY_MODE Texture::TranslateTextureMode(const string mode)
{
	// All comparisons are case-insensitive
	string compare = StrLower(mode);

	if (compare == "repeat")
		return Texture::APPLY_MODE::Repeat;
	else
		return Texture::APPLY_MODE::Normal;
}


void Texture::Shutdown()
{
	// We do not need to release the ID3D11ShaderResourceView texture resource here.  It is deallocated
	// in the shutdown method for the global texture resource repository
	
	return;
}

// Static method that deallocates all textures in the central texture dictionary
void Texture::ShutdownAllTextureData(void)
{
	Texture::TextureResourceRepository::iterator it_end = Texture::TextureResources.end();
	for (Texture::TextureResourceRepository::iterator it = Texture::TextureResources.begin(); it != it_end; ++it)
	{
		if (it->second) 
		{
			// Release the ID3D11ShaderResourceView to deallocate all texture memory
			it->second->Release();
			it->second = NULL;
		}
	}
}



