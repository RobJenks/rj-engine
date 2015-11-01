////////////////////////////////////////////////////////////////////////////////
// Filename: texture.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __TextureH__
#define __TextureH__


//////////////
// INCLUDES //
//////////////
#include <string>
#include <unordered_map>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Utility.h"
#include "CompilerSettings.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Texture
////////////////////////////////////////////////////////////////////////////////
class Texture
{
public:
	enum APPLY_MODE { Normal = 0, Repeat = 1 };

	Texture();
	Texture(const std::string & filename);

	Texture(const Texture&);
	~Texture();

	Result	Initialise(const std::string & filename);
	Result	Initialise(ID3D11Resource *resource, D3D11_SHADER_RESOURCE_VIEW_DESC *resourcedesc);

	CMPINLINE ID3D11ShaderResourceView *GetTexture(void) { return m_texture; };

	void	Shutdown();

	Texture * Clone(void) const;

	CMPINLINE std::string &				GetFilename(void) { return m_filename; }
	CMPINLINE INTVECTOR2				GetTextureSize(void) { return m_size; }
	CMPINLINE void						SetTextureSize(INTVECTOR2 size) { m_size = size; }

	static INTVECTOR2 DetermineTextureSize(ID3D11ShaderResourceView *texture);
	static APPLY_MODE TranslateTextureMode(const string mode);

	// Creates a shader resource view object directly, rather than initialising a texture object
	static ID3D11ShaderResourceView *Texture::CreateSRV(const string & filename);

	// Static method that deallocates all textures in the central texture dictionary
	static void							ShutdownAllTextureData(void);

protected:

	ID3D11ShaderResourceView	*m_texture;
	INTVECTOR2					 m_size;
	std::string					 m_filename;

	// Static controller of texture resources, to avoid having to load multiple instances of the same texture
	typedef std::tr1::unordered_map<std::string, ID3D11ShaderResourceView*>		TextureResourceRepository;
	static TextureResourceRepository											TextureResources;

	// Retrieves a texture resource from the static collection, if a matching resource exists
	CMPINLINE static ID3D11ShaderResourceView *				Get(const string & filename)
	{
		if (Texture::TextureResources.count(filename) > 0)	return Texture::TextureResources[filename];
		else												return NULL;
	}

	// Stores a resource in the static texture collection, assuming it is a valid reference and not a duplicate
	CMPINLINE static void									Store(const string & filename, ID3D11ShaderResourceView *tex)
	{
		if (filename != NullString && tex != NULL && Texture::TextureResources.count(filename) == 0)
			Texture::TextureResources[filename] = tex;
	}
};


#endif