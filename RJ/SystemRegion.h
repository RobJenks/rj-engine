#pragma once

#ifndef __SystemRegionH__
#define __SystemRegionH__

#include "DX11_Core.h"
#include "Rendering.h"
#include "ErrorCodes.h"
class TextureDX11;
class CameraClass;
class ViewFrustrum;
class Model;

// TODO: NOTE THAT, FOR NOW, THIS REGION DOES NOT INHERIT FROM "RegionBase" SINCE IT DOES NOT YET HANDLE ANY SIMULATION
// OF EVENTS IN THE SYSTEM.  WHEN IT DOES, THIS WILL NEED TO HAVE A MIN EXTENT OF THE OUTERMOST REGION IT CONTAINS, AND
// EFFECTIVELY INFINITE MAX BOUNDS SINCE IT WILL BE THE OUTERMOST REGION WITHIN A SYSTEM

// Class is 16-bit aligned to allow use of SIMD member variables (not req now, but will be if inherits from RegionBase in future)
__declspec(align(16))
class SystemRegion : public ALIGN16<SystemRegion>
{
public:
	typedef UINT16 INDEXFORMAT;	

	struct BasicVertexData
	{
		XMFLOAT3 position;
		XMFLOAT3 texture;
	};

	Result				Initialise(void);

	// Sets the system backdrop texture that will be rendered to the view frustrum far plane
	Result				SetBackdropTexture(const std::string & name, const XMFLOAT2 & texturesize );
	
	// Returns a pointer to the texture resource that will be used for rendering the space backdrop
	CMPINLINE bool		HasSystemBackdrop(void) { return m_hasbackdrop; }
	CMPINLINE			Model *GetBackdropSkybox(void) { return m_skybox; }
	ID3D11ShaderResourceView* GetBackdropTextureResource(void);

	// Main render function; renders all applicable objects to the vertex/index buffers
	void				Render(void);

	// Terminates the region
	void				Terminate(void);

	SystemRegion(void);
	~SystemRegion(void);

private:

	// Skybox model & texture resource that will be mapped to it
	bool					m_hasbackdrop;
	Model *					m_skybox;
	TextureDX11 *			m_backdroptexture;

};



#endif