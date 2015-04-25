#pragma once

#ifndef __SystemRegionH__
#define __SystemRegionH__

#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "CubeMapTexture.h"
class CameraClass;
class ViewFrustrum;
class Model;

// TODO: NOTE THAT, FOR NOW, THIS REGION DOES NOT INHERIT FROM "RegionBase" SINCE IT DOES NOT YET HANDLE ANY SIMULATION
// OF EVENTS IN THE SYSTEM.  WHEN IT DOES, THIS WILL NEED TO HAVE A MIN EXTENT OF THE OUTERMOST REGION IT CONTAINS, AND
// EFFECTIVELY INFINITE MAX BOUNDS SINCE IT WILL BE THE OUTERMOST REGION WITHIN A SYSTEM

class SystemRegion
{
public:
	typedef UINT16 INDEXFORMAT;	

	struct BasicVertexData
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

	Result				Initialise(ID3D11Device *device);

	// Sets the system backdrop texture that will be rendered to the view frustrum far plane
	Result				SetBackdropTexture(	ID3D11Device* device, const char *filename, 
											const D3DXVECTOR2 *texturesize );
	
	// Returns a pointer to the texture resource that will be used for rendering the space backdrop
	CMPINLINE bool		HasSystemBackdrop(void) { return m_hasbackdrop; }
	CMPINLINE			Model *GetBackdropSkybox(void) { return m_skybox; }
	CMPINLINE			ID3D11ShaderResourceView* 
								GetBackdropTextureResource(void) { return m_backdroptexture->GetTexture(); }

	// Main render function; renders all applicable objects to the vertex/index buffers
	void				Render(ID3D11DeviceContext *devicecontext);

	void				RenderSystemScenery(ID3D11DeviceContext *devicecontext, const CameraClass *camera, 
											const ViewFrustrum *frustrum, float timefactor);

	// Renders the vertex buffers to the DX output stream, once all data is prepared in the buffers
	void				RenderBuffers(ID3D11DeviceContext *devicecontext);

	// Terminates the region
	void				Terminate(void);

	SystemRegion(void);
	~SystemRegion(void);

private:

	// Skybox model & texture resource that will be mapped to it
	bool					m_hasbackdrop;
	Model					*m_skybox;
	CubeMapTexture			*m_backdroptexture;

};



#endif