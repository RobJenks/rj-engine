#include "DX11_Core.h"

#include <string>
#include "CubeMapTexture.h"
#include "FastMath.h"
#include "CameraClass.h"
#include "ErrorCodes.h"
#include "SystemRegion.h"
#include "Utility.h"
#include "Logging.h"
#include "Model.h"
#include "CoreEngine.h"
#include "GameDataExtern.h"



SystemRegion::SystemRegion(void)
{
	// Initialise pointers to NULL and variables to defaults
	m_hasbackdrop = false;
	m_skybox = NULL;
	m_backdroptexture = NULL;
}

Result SystemRegion::Initialise(void)
{
	// Create a new model to represent the space skybox
	m_skybox = Model::GetModel("skybox");
	if (!m_skybox)
	{
		Game::Log << LOG_WARN << "Cannot load geometry for system skybox\n";
	}

	// Return success following initialisation
	return ErrorCodes::NoError;
}

ID3D11ShaderResourceView * SystemRegion::GetBackdropTextureResource(void)
{
	return (m_backdroptexture ? m_backdroptexture->GetShaderResourceView() : NULL);
}

Result SystemRegion::SetBackdropTexture(const std::string & name, const XMFLOAT2 & texturesize)
{
	m_backdroptexture = Game::Engine->GetAssets().GetTexture(name);
	m_hasbackdrop = true;

	if (!m_backdroptexture)
	{
		m_hasbackdrop = false;
		Game::Log << LOG_WARN << "Cannot load skybox texture \"" << name << "\"\n";
	}

	return ErrorCodes::NoError;
}

void SystemRegion::Render(void)
{
	// Render the skybox
	//m_skybox->Render();	// TODO: Re-enable
}


void SystemRegion::Terminate(void)
{
}

SystemRegion::~SystemRegion(void)
{
}
