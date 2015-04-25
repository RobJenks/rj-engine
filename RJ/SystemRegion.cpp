#include "DX11_Core.h"

#include <string>
#include "CubeMapTexture.h"
#include "FastMath.h"
#include "CameraClass.h"
#include "ViewFrustrum.h"
#include "ErrorCodes.h"
#include "SystemRegion.h"
#include "Utility.h"
#include "Model.h"
#include "GameDataExtern.h"
using namespace std;

Result SystemRegion::Initialise(ID3D11Device *device)
{
	Result result;

	// Create a new model to represent the space skybox
	string skybox_filename = BuildStrFilename(D::DATA, "Models\\Misc\\skybox.rjm");
	m_skybox = new Model();

	// Initialise the skybox model
	result = m_skybox->Initialise(skybox_filename.c_str(), NULL);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success following initialisation
	return ErrorCodes::NoError;
}

Result SystemRegion::SetBackdropTexture(ID3D11Device* device, const char *filename, const D3DXVECTOR2 *texturesize)
{
	// If we already have a texture set then release and deallocate it first
	m_hasbackdrop = false;
	if (m_backdroptexture) {
		m_backdroptexture->Shutdown();
		delete m_backdroptexture; 
		m_backdroptexture = NULL;
	}

	// Create the texture object
	m_backdroptexture = new CubeMapTexture(); 
	if(!m_backdroptexture) return ErrorCodes::CouldNotCreateTextureObject;

	// Initialize the texture object
	Result result = m_backdroptexture->Initialise(device, filename);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success and set the backdrop flag if we loaded the texture successfully
	m_hasbackdrop = true;
	return ErrorCodes::NoError;
}

void SystemRegion::Render(ID3D11DeviceContext *devicecontext)
{
	// Render the skybox
	m_skybox->Render();
}


SystemRegion::SystemRegion(void)
{
	// Initialise pointers to NULL and variables to defaults
	m_hasbackdrop = false;
	m_skybox = NULL;
	m_backdroptexture = NULL;
}

void SystemRegion::Terminate(void)
{
	// Deallocate memory used for the system region
	if (m_backdroptexture) { m_backdroptexture->Shutdown(); delete m_backdroptexture; m_backdroptexture = NULL; }
	if (m_skybox) { m_skybox->Shutdown(); delete m_skybox; m_skybox = NULL; }
}

SystemRegion::~SystemRegion(void)
{
}
