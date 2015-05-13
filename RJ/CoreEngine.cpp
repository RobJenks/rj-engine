#include "DX11_Core.h" // #include "FullDX11.h"

#include "ErrorCodes.h"
#include "GlobalFlags.h"
#include "D3DMain.h"
#include "Profiler.h"
#include "CameraClass.h"
#include "LightShader.h"
#include "LightFadeShader.h"
#include "LightHighlightShader.h"
#include "LightHighlightFadeShader.h"
#include "ParticleShader.h"
#include "TextureShader.h"
#include "TexcubeShader.h"
#include "FireShader.h"
#include "Light.h"
#include "ViewFrustrum.h"
#include "BoundingObject.h"
#include "FontShader.h"
#include "TextManager.h"
#include "EffectManager.h"
#include "ParticleEngine.h"
#include "Render2DManager.h"
#include "SkinnedNormalMapShader.h"
#include "OverlayRenderer.h"

#include "Fonts.h"
#include "Player.h"
#include "Model.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "iContainsComplexShipTiles.h"
#include "ComplexShipTile.h"
#include "FastMath.h"
#include "Utility.h"
#include "GameDataExtern.h"
#include "SimulationObjectManager.h"
#include "SpaceSystem.h"
#include "ImmediateRegion.h"
#include "SystemRegion.h"
#include "UserInterface.h"
#include "SkinnedModel.h"
#include "StaticTerrainDefinition.h"
#include "Actor.h"
#include "GameConsoleCommand.h"
#include <tchar.h>
#include <unordered_map>

#include "CoreEngine.h"

using namespace std;
using namespace std::tr1;


CoreEngine::CoreEngine(void)
{
	// Reset all component pointers to NULL, in advance of initialisation
	m_dxlocaliser = NULL;
	m_D3D = NULL;
	m_camera = NULL;
	m_lightshader = NULL;
	m_lightfadeshader = NULL;
	m_lighthighlightshader = NULL;
	m_lighthighlightfadeshader = NULL;
	m_light = NULL;
	m_particleshader = NULL;
	m_textureshader = NULL;
	m_texcubeshader = NULL;
	m_frustrum = NULL;
	m_textmanager = NULL;
	m_fontshader = NULL;
	m_fireshader = NULL;
	m_skinnedshader = NULL;
	m_effectmanager = NULL;
	m_particleengine = NULL;
	m_render2d = NULL;
	m_overlayrenderer = NULL;
	m_instancebuffer = NULL;
	m_debug_renderenvboxes = 0;
	
	// Set default values for game engine parameters
	m_hwnd = NULL;
	m_vsync = false;

	// Initialise all render stage flags to true at startup
	m_renderstages = std::vector<bool>(CoreEngine::RenderStage::Render_STAGECOUNT, true);
	
	// Initialise all special render flags at startup
	m_renderflags = std::vector<bool>(CoreEngine::RenderFlag::_RFLAG_COUNT, false);

	// Set pre-populated parameter values for render-time efficiency
	m_instanceparams = NULL_VECTOR4;										// x component holds fade alpha

	// Initialise all temporary/cache fields that are used for more efficient intermediate calculations
	m_cache_zeropoint = m_cache_el_inc[0] = m_cache_el_inc[1] = m_cache_el_inc[2] = NULL_VECTOR;
	m_tmp_vector3 = NULL_VECTOR;
	m_tmp_matrix = ID_MATRIX;

	// Initialise the cached increment vectors, representing a +1 element movement in each dimension in turn
	m_cache_el_inc_base[0] = D3DXVECTOR3(Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	m_cache_el_inc_base[1] = D3DXVECTOR3(0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f);
	m_cache_el_inc_base[2] = D3DXVECTOR3(0.0f, 0.0f, Game::C_CS_ELEMENT_SCALE);
}


Result CoreEngine::InitialiseGameEngine(HWND hwnd)
{ 
	Result res;

	// Initialise each component in turn; in case of failure, attempt to roll back anything possible and return an error
	Game::Log << "\n" << LOG_INIT_START << "Beginning initialisation of game engine\n";

	// Initialise all render flags to their default values
	res = InitialiseRenderFlags();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Render flags initialised\n";

	// Initialise the DX localiser, which will configure the D3D etc that is subsequently set up
	res = InitialiseDXLocaliser(DXLocaliser::DXLevel::DirectX_11_0);
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "DX Localiser initialised\n";

	// Initialise the Direct3D component
	res = InitialiseDirect3D(hwnd);
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Direct3D initialisation complete\n";

	// Initialise the camera component
	res = InitialiseCamera();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Camera initialised\n";

	// Initialise the light objects
	res = InitialiseLights();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Lighting initialisation complete\n";

	// Initialise the light shader
	res = InitialiseLightShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Light] initialisation complete\n";

	// Initialise the light/fade shader
	res = InitialiseLightFadeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Light fade] initialisation complete\n";

	// Initialise the light/highlight shader
	res = InitialiseLightHighlightShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Light highlight] initialisation complete\n";

	// Initialise the light/highlight/fade shader
	res = InitialiseLightHighlightFadeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Light highlight fade] initialisation complete\n";

	// Initialise the particle shader
	res = InitialiseParticleShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Particle] initialisation complete\n";

	// Initialise the texture shader
	res = InitialiseTextureShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Texture] initialisation complete\n";

	// Initialise the view frustrum
	res = InitialiseFrustrum();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "View frustum created\n";

	// Initialise the font shader
	res = InitialiseFontShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Font] initialisation complete\n";

	// Initialise the text rendering components
	res = InitialiseTextRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Text rendering initialised\n";

	// Initialise all game fonts
	res = InitialiseFonts();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Font initialisation complete\n";

	// Initialise the texcube shader
	res = InitialiseTexcubeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Texcube] initialisation complete\n";

	// Initialise the fire shader
	res = InitialiseFireShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Fire] initialisation complete\n";

	// Initialise the effect manager
	res = InitialiseEffectManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Effect manager initialised\n";

	// Initialise the skinned normal map shader
	res = InitialiseSkinnedNormalMapShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Shader [Skinned normal map] initialisation complete\n";

	// Initialise the particle engine
	res = InitialiseParticleEngine();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Particle engine initialised\n";

	// Initialise the 2D render manager
	res = Initialise2DRenderManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "2D render manager initialisation complete\n";

	// Initialise the overlay renderer
	res = InitialiseOverlayRenderer();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Overlay renderer initialised\n";
	
	// Initialise the render queue for geometry instancing & batching (dependent on initialisation of relevant shaders)
	res = InitialiseRenderQueue();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Render queue initialised for all shaders\n";

	// Initialise the components used for environment rendering
	res = InitialiseEnvironmentRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INIT_START << "Environment rendering initialised\n";


	// If we succeed in all initialisation functions then return success now
	Game::Log << LOG_INIT_START << "All game engine initialisation completed successfully\n\n";
	return ErrorCodes::NoError;
}

void CoreEngine::ShutdownGameEngine()
{
	// Run the termination function for each component
	ShutdownLightShader();
	ShutdownLightFadeShader();
	ShutdownLightHighlightShader();
	ShutdownLightHighlightFadeShader();
	ShutdownParticleShader();
	ShutdownTextureShader();
	ShutdownLights();
	ShutdownCamera();
	ShutdownFrustrum();
	ShutdownTextRendering();
	ShutdownFontShader();
	ShutdownFonts();
	ShutdownTexcubeShader();
	ShutdownFireShader();
	ShutdownEffectManager();
	ShutdownSkinnedNormalMapShader();
	ShutdownParticleEngine();
	Shutdown2DRenderManager();
	ShutdownOverlayRenderer();
	ShutdownRenderQueue();
	ShutdownEnvironmentRendering();
	ShutdownTextureData();
	ShutdownDirect3D();
	ShutdownDXLocaliser();	
}

Result CoreEngine::InitialiseRenderFlags(void)
{
	// Initialise all render flags to default values
	
	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseDirect3D(HWND hwnd)
{
	// Store key window parameters
	m_hwnd = hwnd; 

	// Attempt to create the Direct3D object.
	m_D3D = new D3DMain(m_dxlocaliser);
	if ( !m_D3D ) return ErrorCodes::CannotCreateDirect3DDevice;

	// Initialise the Direct3D object.
	Result result = m_D3D->Initialise(Game::ScreenWidth, Game::ScreenHeight, m_vsync, m_hwnd, Game::FullScreen, 
									  SCREEN_DEPTH, SCREEN_NEAR);
	return result;
}

Result CoreEngine::InitialiseDXLocaliser(DXLocaliser::DXLevel DirectXLevel)
{
	// Attempt to create a new DX Localiser instance
	m_dxlocaliser = new DXLocaliser();
	if ( !m_dxlocaliser ) return ErrorCodes::CannotCreateDXLocaliserComponent;

	// Initialise the localiser with the supplied feature level
	Result result = m_dxlocaliser->Initialise(DirectXLevel);
	return result;
}

Result CoreEngine::InitialiseRenderQueue(void)
{
	D3D11_BUFFER_DESC ibufdesc;
	D3D11_SUBRESOURCE_DATA ibufdata;

	// Create a new empty instance stream, for initialisation of the buffer
	RM_Instance *idata = (RM_Instance*)malloc(Game::C_INSTANCED_RENDER_LIMIT * sizeof(RM_Instance));
	memset(idata, 0, Game::C_INSTANCED_RENDER_LIMIT * sizeof(RM_Instance));

	// Create the instance buffer description
	ibufdesc.Usage = D3D11_USAGE_DYNAMIC;
	ibufdesc.ByteWidth = sizeof(RM_Instance) * Game::C_INSTANCED_RENDER_LIMIT;
	ibufdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	ibufdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibufdesc.MiscFlags = 0;
	ibufdesc.StructureByteStride = 0;

	// Link the subresource object to the initial empty instance stream
	ibufdata.pSysMem = idata;
	ibufdata.SysMemPitch = 0;
	ibufdata.SysMemSlicePitch = 0;

	// Create the instance buffer
	HRESULT hr = m_D3D->GetDevice()->CreateBuffer(&ibufdesc, &ibufdata, &m_instancebuffer);
	if (FAILED(hr)) return ErrorCodes::CouldNotInitialiseInstanceBuffer;

	// Release memory used to initialise the instance buffer
	free(idata); idata = NULL;

	// Initialise the buffer pointers, stride and offset values
	m_instancedbuffers[0] = NULL;									// Buffer[0] will be populated with each VB
	m_instancedbuffers[1] = m_instancebuffer;		
	m_instancedstride[0] = Model::GetVertexMemorySize();
	m_instancedstride[1] = sizeof(RM_Instance);
	m_instancedoffset[0] = 0; m_instancedoffset[1] = 0;

	// Initialise the render queue with a blank map for each shader in scope
	m_renderqueue = CoreEngine::RM_ShaderModelInstanceMap(CoreEngine::RenderQueueShader::RM_RENDERQUEUESHADERCOUNT);
	m_renderqueueshaders = CoreEngine::RM_ShaderCollection(CoreEngine::RenderQueueShader::RM_RENDERQUEUESHADERCOUNT);

	// Set the reference and parameters for each shader in turn
	m_renderqueueshaders[CoreEngine::RenderQueueShader::RM_LightShader] = 
		RM_InstancedShaderDetails((iShader*)m_lightshader, false, D3DMain::AlphaBlendState::AlphaBlendDisabled);
	m_renderqueueshaders[CoreEngine::RenderQueueShader::RM_LightHighlightShader] = 
		RM_InstancedShaderDetails((iShader*)m_lighthighlightshader, false, D3DMain::AlphaBlendState::AlphaBlendDisabled);
	m_renderqueueshaders[CoreEngine::RenderQueueShader::RM_LightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lightfadeshader, true, D3DMain::AlphaBlendState::AlphaBlendEnabledNormal);
	m_renderqueueshaders[CoreEngine::RenderQueueShader::RM_LightHighlightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lighthighlightfadeshader, true, D3DMain::AlphaBlendState::AlphaBlendEnabledNormal);

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseCamera(void)
{
	D3DXVECTOR3 pos;

	// Attempt to create a new camera instance
	m_camera = new CameraClass();
	if ( !m_camera ) return ErrorCodes::CannotCreateCameraComponent;

	// Initialise the camera
	Result result = m_camera->Initialise();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Use the camera rendering functions to generate a temporary view matrix for all 2D text rendering
	pos = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	m_camera->CalculateViewMatrixFromPositionData(&pos, &ID_QUATERNION, &ID_MATRIX);
	m_camera->GetViewMatrix(m_baseviewmatrix);

	// Return success
	return ErrorCodes::NoError;

}

Result CoreEngine::InitialiseLightShader(void)
{
	// Create the light shader object.
	m_lightshader = new LightShader(m_dxlocaliser);
	if(!m_lightshader)
	{
		return ErrorCodes::CouldNotCreateLightShader;
	}

	// Initialise the light shader object.
	Result result = m_lightshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// For now, set the light parameters as per the defined light objects
	m_lightshader->SetLightParameters(m_light->GetDirection(), m_light->GetAmbientColor(), m_light->GetDiffuseColor());

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseLightFadeShader(void)
{
	// Create the light shader object.
	m_lightfadeshader = new LightFadeShader(m_dxlocaliser);
	if(!m_lightfadeshader)
	{
		return ErrorCodes::CouldNotCreateLightFadeShader;
	}

	// Initialise the light shader object.
	Result result = m_lightfadeshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// For now, set the light parameters as per the defined light objects
	m_lightfadeshader->SetLightParameters(m_light->GetDirection(), m_light->GetAmbientColor(), m_light->GetDiffuseColor());

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}


Result CoreEngine::InitialiseLightHighlightShader(void)
{
	// Create the light shader object.
	m_lighthighlightshader = new LightHighlightShader(m_dxlocaliser);
	if(!m_lighthighlightshader)
	{
		return ErrorCodes::CouldNotCreateLightHighlightShader;
	}

	// Initialise the light shader object.
	Result result = m_lighthighlightshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// For now, set the light parameters as per the defined light objects
	m_lighthighlightshader->SetLightParameters(m_light->GetDirection(), m_light->GetAmbientColor(), m_light->GetDiffuseColor());

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}


Result CoreEngine::InitialiseLightHighlightFadeShader(void)
{
	// Create the light shader object.
	m_lighthighlightfadeshader = new LightHighlightFadeShader(m_dxlocaliser);
	if (!m_lighthighlightfadeshader)
	{
		return ErrorCodes::CouldNotCreateLightHighlightFadeShader;
	}

	// Initialise the light shader object.
	Result result = m_lighthighlightfadeshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// For now, set the light parameters as per the defined light objects
	m_lighthighlightfadeshader->SetLightParameters(m_light->GetDirection(), m_light->GetAmbientColor(), m_light->GetDiffuseColor());

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}


Result CoreEngine::InitialiseParticleShader(void)
{
	// Create the particle shader object
	m_particleshader = new ParticleShader(m_dxlocaliser);
	if (!m_particleshader)
	{
		return ErrorCodes::CouldNotCreateParticleShader;
	}

	// Initialise the particle shader
	Result result = m_particleshader->Initialize(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we got this far
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseTextureShader(void)
{
	// Create the texture shader object
	m_textureshader = new TextureShader(m_dxlocaliser);
	if (!m_textureshader)
	{
		return ErrorCodes::CouldNotCreateTextureShader;
	}

	// Initialise the particle shader
	Result result = m_textureshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we got this far
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseLights()
{
	// Create the light object.
	m_light = new Light();
	if(!m_light)
	{
		return ErrorCodes::CannotCreateLightObject;
	}

	// Initialise the light object.
	m_light->SetAmbientColor(0.25f, 0.25f, 0.25f, 1.0f);
	m_light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f); 
	m_light->SetDirection(0.0f, 0.0f, 1.0f);

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseFrustrum()
{
	// Attempt to create a new view frustrum
	m_frustrum = new ViewFrustrum();
	if (!m_frustrum) return ErrorCodes::CannotCreateViewFrustrum;
	
	// Get a reference to the projection matrix for the initialisation calculations
	D3DXMATRIX proj;
	m_D3D->GetProjectionMatrix(proj);

	// Run the initialisation function with viewport/projection data that can be precaulcated
	Result res = m_frustrum->Initialise(&proj, SCREEN_DEPTH, m_D3D->GetDisplayFOV(), m_D3D->GetDisplayAspectRatio());
	if (res != ErrorCodes::NoError) return res;	

	// Return success if the frustrum was created
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseFontShader(void)
{
	Result result;

	// Create the font shader object.
	m_fontshader = new FontShader(m_dxlocaliser);
	if(!m_fontshader)
	{
		return ErrorCodes::CannotCreateFontShader;
	}

	// Initialize the font shader object.
	result = m_fontshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseTextRendering(void)
{
	Result result;
	D3DXVECTOR3 pos;

	// Create the text manager object
	m_textmanager = new TextManager(m_dxlocaliser);
	if (!m_textmanager)
	{
		return ErrorCodes::CannotCreateTextRenderer;
	}

	// Now attempt to initialise the text rendering object
	result = m_textmanager->Initialize(	m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_hwnd, 
										Game::ScreenWidth, Game::ScreenHeight, m_baseviewmatrix, m_fontshader );
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;

}

Result CoreEngine::InitialiseFonts(void)
{
	Result result;

	// Make sure the text manager has been successfully initialised first
	if (!m_textmanager) return ErrorCodes::CannotInitialiseFontsWithoutTextManager;

	// Initialise each font in turn from its data file and composite texture
	// TODO: To be loaded from external file?  How to set Game::Fonts::* IDs?
	result = m_textmanager->InitializeFont("Font_Basic1", 
				"../RJ/Data/Fonts/font_basic1.txt", "../RJ/Data/Fonts/font_basic1.dds", Game::Fonts::FONT_BASIC1);
	if (result != ErrorCodes::NoError) return result;

	// Return success when all fonts are loaded
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseTexcubeShader(void)
{
	// Create the texture shader object
	m_texcubeshader = new TexcubeShader(m_dxlocaliser);
	if (!m_texcubeshader)
	{
		return ErrorCodes::CouldNotCreateTexcubeShader;
	}

	// Initialise the particle shader
	Result result = m_texcubeshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we got this far
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseFireShader(void)
{
	// Create the fire shader object
	m_fireshader = new FireShader(m_dxlocaliser);
	if (!m_fireshader)
	{
		return ErrorCodes::CouldNotCreateFireShader;
	}

	// Initialise the fire shader
	Result result = m_fireshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we got this far
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseEffectManager(void)
{
	Result result;

	// Create the fire shader object
	m_effectmanager = new EffectManager(m_dxlocaliser);
	if (!m_effectmanager)
	{
		return ErrorCodes::CouldNotCreateEffectManager;
	}

	// Initialise the effect manager
	result = m_effectmanager->Initialise(m_D3D->GetDevice());
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Now attempt to link all required shaders to the effect manager.  Dependency: must be called after all shaders initialised
	result = m_effectmanager->LinkEffectShaders(m_fireshader);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we got this far
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseSkinnedNormalMapShader(void)
{
	// Create a new instance of the shader object
	m_skinnedshader = new SkinnedNormalMapShader(m_dxlocaliser);
	if (!m_skinnedshader)
	{
		return ErrorCodes::CannotCreateSkinnedNormalMapShader;
	}

	// Now attempt to initialise the shader
	Result result = m_skinnedshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseParticleEngine(void)
{
	Result result;

	// Create the particle engine object
	m_particleengine = new ParticleEngine(m_dxlocaliser);
	if (!m_particleengine)
	{
		return ErrorCodes::CouldNotCreateParticleEngine;
	}

	// Attempt to initialise the particle engine
	result = m_particleengine->Initialise();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Now link all required shaders to the particle engine.  Dependency: must be called after all shaders initialised
	result = m_particleengine->LinkParticleShader(m_particleshader);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::Initialise2DRenderManager(void)
{
	// Create a new 2D render manager instance
	m_render2d = new Render2DManager(m_dxlocaliser);
	if (!m_render2d)
	{
		return ErrorCodes::CannotCreate2DRenderManager;
	}

	// Now attempt to initialise the render manager
	Result result = m_render2d->Initialise(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_hwnd, 
										   Game::ScreenWidth, Game::ScreenHeight, m_baseviewmatrix);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseOverlayRenderer(void)
{
	// Create a new overlay render manager
	m_overlayrenderer = new OverlayRenderer();
	if (!m_overlayrenderer)
	{
		return ErrorCodes::CannotCreateOverlayRenderer;
	}

	// Now attempt to initialise the overlay renderer
	Result result = m_overlayrenderer->Initialise();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseEnvironmentRendering(void)
{
	// Pre-allocate space in the temporary rendering vectors to limit the amount of reallocation required at runtime
	m_tmp_renderedtiles.reserve(1000);
	m_tmp_renderedobjects.reserve(2000);
	m_tmp_renderedterrain.reserve(3000);

	// Return success
	return ErrorCodes::NoError;
}


void CoreEngine::ShutdownDirect3D(void)
{
	// Attempt to release the Direct3D component
	if ( m_D3D )
	{
		m_D3D->Shutdown();
		delete m_D3D; m_D3D = NULL;
	}
}

void CoreEngine::ShutdownDXLocaliser(void)
{
	// Attempt to release the localiser component
	if ( m_dxlocaliser )
	{
		delete m_dxlocaliser; 
		m_dxlocaliser = NULL;
	}
}

void CoreEngine::ShutdownRenderQueue(void)
{
	// Clear the render queue
	if (m_renderqueue.size() > 0)
	{
		ClearRenderingQueue();
	}

	// Free all resources held by the instance buffer
	if ( m_instancebuffer )
	{
		m_instancebuffer->Release(); 
		m_instancebuffer = NULL;
	}
}

void CoreEngine::ShutdownTextureData(void)
{	
	// Shutdown all texture data in the static global collection
	Texture::ShutdownAllTextureData();
}

void CoreEngine::ShutdownCamera(void)
{
	// Attempt to release the camera component
	if ( m_camera )
	{
		delete m_camera; 
		m_camera = NULL;
	}
}

void CoreEngine::ShutdownLightShader(void)
{
	// Release the light shader object.
	if(m_lightshader)
	{
		m_lightshader->Shutdown();
		delete m_lightshader;
		m_lightshader = NULL;
	}
}

void CoreEngine::ShutdownLightFadeShader(void)
{
	// Release the light shader object.
	if(m_lightfadeshader)
	{
		m_lightfadeshader->Shutdown();
		delete m_lightfadeshader;
		m_lightfadeshader = NULL;
	}
}

void CoreEngine::ShutdownLightHighlightShader(void)
{
	// Release the light shader object.
	if(m_lighthighlightshader)
	{
		m_lighthighlightshader->Shutdown();
		delete m_lighthighlightshader;
		m_lighthighlightshader = NULL;
	}
}

void CoreEngine::ShutdownLightHighlightFadeShader(void)
{
	// Release the light shader object.
	if (m_lighthighlightfadeshader)
	{
		m_lighthighlightfadeshader->Shutdown();
		delete m_lighthighlightfadeshader;
		m_lighthighlightfadeshader = NULL;
	}
}

void CoreEngine::ShutdownParticleShader(void)
{
	if (m_particleshader)
	{
		m_particleshader->Shutdown();
		delete m_particleshader;
		m_particleshader = NULL;
	}
}

void CoreEngine::ShutdownTextureShader(void)
{
	if (m_textureshader)
	{
		m_textureshader->Shutdown();
		delete m_textureshader;
		m_textureshader = NULL;
	}
}


void CoreEngine::ShutdownLights(void)
{
	// Release the light object.
	if(m_light)
	{
		delete m_light;
		m_light = 0;
	}
}

void CoreEngine::ShutdownFrustrum(void)
{
	// Release the view frustrum object.
	if(m_frustrum)
	{
		delete m_frustrum;
		m_frustrum = 0;
	}
}

void CoreEngine::ShutdownTextRendering(void)
{
	// Release the text manager object.
	if(m_textmanager)
	{
		m_textmanager->Shutdown();
		delete m_textmanager;
		m_textmanager = 0;
	}
}

void CoreEngine::ShutdownFontShader(void)
{
	// Release the font shader object.
	if(m_fontshader)
	{
		m_fontshader->Shutdown();
		delete m_fontshader;
		m_fontshader = 0;
	}
}

void CoreEngine::ShutdownTexcubeShader(void)
{
	if (m_texcubeshader)
	{
		m_texcubeshader->Shutdown();
		delete m_texcubeshader;
		m_texcubeshader = NULL;
	}
}

void CoreEngine::ShutdownFireShader(void)
{
	if (m_fireshader)
	{
		m_fireshader->Shutdown();
		delete m_fireshader;
		m_fireshader = NULL;
	}
}

void CoreEngine::ShutdownFonts(void)
{
	// No actions to be taken; fonts are deallocated as part of the text manager shutdown
}

void CoreEngine::ShutdownEffectManager(void)
{
	if (m_effectmanager)
	{
		m_effectmanager->Shutdown();
		delete m_effectmanager;
		m_effectmanager = NULL;
	}
}

void CoreEngine::ShutdownSkinnedNormalMapShader(void)
{
	if (m_skinnedshader)
	{
		m_skinnedshader->Shutdown();
		delete m_skinnedshader;
		m_skinnedshader = NULL;
	}
}

void CoreEngine::ShutdownParticleEngine(void)
{
	if (m_particleengine)
	{
		m_particleengine->Shutdown();
		delete m_particleengine;
		m_particleengine = NULL;
	}
}

void CoreEngine::Shutdown2DRenderManager(void)
{
	if (m_render2d)
	{
		m_render2d->Shutdown();
		delete m_render2d;
		m_render2d = NULL;
	}
}

void CoreEngine::ShutdownOverlayRenderer(void)
{
	if (m_overlayrenderer)
	{
		m_overlayrenderer->Shutdown();
		delete m_overlayrenderer;
		m_overlayrenderer = NULL;
	}
}

void CoreEngine::ShutdownEnvironmentRendering(void)
{
	// No action required, for now
}


// The main rendering function; renders everything in turn as required
void CoreEngine::Render(void)
{
	// Reset render statistics ready for the next frame
	ResetRenderInfo();

	// Retrieve render-cycle-specific data that will not change for the duration of the cycle.  Prefixed r_*
	r_devicecontext = m_D3D->GetDeviceContext();
	m_D3D->GetProjectionMatrix(r_projection);
	m_camera->GetViewMatrix(r_view);
	m_camera->GetInverseViewMatrix(r_invview);
	m_D3D->GetOrthoMatrix(r_orthographic);

	// Validate render cycle parameters before continuing with the render process
	if (!r_devicecontext) return;

	// Construct the view frustrum for this frame so we can perform culling calculations
	m_frustrum->ConstructFrustrum(&r_view, &r_invview);

	/*** Perform rendering that is common to all player environment & states ***/

	// Render the system region
	if (RenderStageActive(RenderStage::Render_SystemRegion)) 
		RenderSystemRegion();			// The system-wide rendering, e.g. space backdrop and scenery

	// Render the immmediate player region, e.g. localised space dust
	if (RenderStageActive(RenderStage::Render_ImmediateRegion)) 
		RenderImmediateRegion();

	// Render all objects in the current player system; simulation state & visibility will be taken 
	// into account to ensure we only render those items that are necessary
	if (RenderStageActive(RenderStage::Render_SystemObjects)) 
		RenderAllSystemObjects(Game::CurrentPlayer->GetSystem());

	// Render effects and particle emitters
	if (RenderStageActive(RenderStage::Render_Effects)) RenderEffects();
	if (RenderStageActive(RenderStage::Render_ParticleEmitters)) RenderParticleEmitters();

	// Perform all 2D rendering of text and UI components
	if (RenderStageActive(RenderStage::Render_UserInterface))
		RenderUserInterface();	

	// Perform any debug/special rendering 
	if (RenderStageActive(RenderStage::Render_DebugData))
		RenderDebugData();

	// Final activity: process all items queued for rendering.  Any item that benefits from instanced/batched geometry rendering is
	// added to the render queue during the process above.  We now use instanced rendering on the entire render queue.  The more 
	// high-volume items that can be moved to use instanced rendering the better
	ProcessRenderQueue();
}

// Processes all items in the render queue using instanced rendering, to minimise the number of render calls required per frame
RJ_PROFILED(void CoreEngine::ProcessRenderQueue, void)
{
	RM_ShaderModelInstanceMap::iterator smi, smi_end;
	RM_ModelInstanceMap::iterator mi, mi_end;
	D3D11_MAPPED_SUBRESOURCE mappedres;
	int instancecount, inst, n;

	// Iterate through each shader in the render queue
	for (int i = 0; i < CoreEngine::RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; i++)
	{
		// Set any engine properties required by this specific shader
		if (m_renderqueueshaders[i].AlphaBlendRequired != m_D3D->GetAlphaBlendState())	
			m_D3D->SetAlphaBlendState(m_renderqueueshaders[i].AlphaBlendRequired);

		// If this is a shader that requires z-sorting, perform that sort and render by the sorted method.  We can 
		// then skip the remainder of the process for this shader and move on to the next one
		if (m_renderqueueshaders[i].RequiresZSorting) { PerformZSortedRenderQueueProcessing(i); continue; }

		// Iterate through each model queued for rendering by this shader
		mi_end = m_renderqueue[i].end();
		for (mi = m_renderqueue[i].begin(); mi != mi_end; ++mi)
		{
			// Get the number of instances to be rendered
			instancecount = mi->second.size();
			if (instancecount == 0) continue;

			// Loop through the instances in batches, if the total count is larger than our limit
			for (inst = 0; inst < instancecount; inst += Game::C_INSTANCED_RENDER_LIMIT)
			{
				// Determine the number of instances to render; either the per-batch limit, or fewer if we do not have that many
				n = min(instancecount - inst, Game::C_INSTANCED_RENDER_LIMIT);

				// Update the instance buffer by mapping, updating and unmapping the memory
				memset(&mappedres, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
				r_devicecontext->Map(m_instancebuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedres);
				memcpy(mappedres.pData, &(mi->second[inst]), sizeof(RM_Instance) * n);
				r_devicecontext->Unmap(m_instancebuffer, 0);

				// Update the model VB pointer and then set vertex buffer data
				m_instancedbuffers[0] = mi->first->GetVertexBuffer();
				r_devicecontext->IASetVertexBuffers(0, 2, m_instancedbuffers, m_instancedstride, m_instancedoffset);

				// Set the model index buffer to active in the input assembler
				r_devicecontext->IASetIndexBuffer(mi->first->GetIndexBuffer(), /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

				// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
				r_devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// Now process all instanced / indexed vertex data through this shader
				m_renderqueueshaders[i].Shader->Render(	r_devicecontext, mi->first->GetIndexCount(), mi->first->GetIndexCount(), n, 
														r_view, r_projection, mi->first->GetTexture());

				// Increment the count of draw calls that have been processed
				++m_renderinfo.DrawCalls;
			}

			// Finally, clear the instance vector for this shader/model now that we have fully processed it
			mi->second.clear();
		}
	}	

	// Return any render parameters to their default if required, to avoid any downstream impact
	if (m_D3D->GetAlphaBlendState() != D3DMain::AlphaBlendState::AlphaBlendDisabled) m_D3D->SetAlphaBlendModeDisabled();
}

// Performs an intermediate z-sorting of instances before populating and processing the render queue.  Used only for 
// shaders/techniques (e.g. alpha blending) that require instances to be z-sorted.  Takes the place of normal rendering
void CoreEngine::PerformZSortedRenderQueueProcessing(int shaderindex)
{
	const Model *model = NULL; int n;
	vector<RM_Instance> renderbuffer;
	D3D11_MAPPED_SUBRESOURCE mappedres;

	// Sort the vector by z-order.  Uses default "operator<" defined in the RM_ZSortedInstance struct
	std::sort(m_renderqueueshaders[shaderindex].SortedInstances.begin(), m_renderqueueshaders[shaderindex].SortedInstances.end());

	// Now reverse iterate through the newly-sorted items in the vector, to pull instances in decreasing distance from the camera
	// Deliberately go to -1, so we can render the final element(s).  Loop will run from (n-1) to -1
	int size = m_renderqueueshaders[shaderindex].SortedInstances.size();
	if (size > 0)
	{
		// The starting model will be that of the first element (which we know exists since size>0)
		model = m_renderqueueshaders[shaderindex].SortedInstances[size-1].ModelPtr;
		for (int i = size-1; i >= -1; --i)									
		{
			// If this is an instance of the same model as the previous item, and this is not the final (-1) dummy item,
			// add another element to the render buffer
			if (i != -1 && m_renderqueueshaders[shaderindex].SortedInstances[i].ModelPtr == model)
			{
				renderbuffer.push_back(m_renderqueueshaders[shaderindex].SortedInstances[i].Item);
			}

			// If this is an instance of a different model, or is the dummy end-element, we want to render the buffer that has been accumulated so far
			if (i == -1 || m_renderqueueshaders[shaderindex].SortedInstances[i].ModelPtr != model)
			{
				// We are at this point because (a) we are at the end of the vector, or (b) the model has changed for a valid reason
				// We therefore want to render the buffer now.  Make sure that the buffer actually contains items 
				n = renderbuffer.size();
				if (n > 0)
				{
					// Make sure we are not over the instance limit.  If we are, simply truncate.  Should not be rendering many items this way anyway
					n = min(n, Game::C_INSTANCED_RENDER_LIMIT);

					// Update the instance buffer by mapping, updating and unmapping the memory
					memset(&mappedres, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
					r_devicecontext->Map(m_instancebuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedres);
					memcpy(mappedres.pData, &(renderbuffer[0]), sizeof(RM_Instance) * n);
					r_devicecontext->Unmap(m_instancebuffer, 0);

					// Update the model VB pointer and then set vertex buffer data
					m_instancedbuffers[0] = model->GetVertexBuffer();
					r_devicecontext->IASetVertexBuffers(0, 2, m_instancedbuffers, m_instancedstride, m_instancedoffset);

					// Set the model index buffer to active in the input assembler
					r_devicecontext->IASetIndexBuffer(model->GetIndexBuffer(), /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

					// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
					r_devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					// Now process all instanced / indexed vertex data through this shader
					m_renderqueueshaders[shaderindex].Shader->Render(	r_devicecontext, model->GetIndexCount(), model->GetIndexCount(), n, 
																		r_view, r_projection, model->GetTexture());

					// Increment the count of draw calls that have been processed
					++m_renderinfo.DrawCalls;
				}
			
				// Clear the render buffer now that it has been rendered
				renderbuffer.clear();

				// We are either here because the model has changed, or we are at the -1 element.  If we are not at the -1 element
				// we now want to update the current model pointer, and add the current element to the render buffer as the first item
				if (i != -1)
				{
					model = m_renderqueueshaders[shaderindex].SortedInstances[i].ModelPtr;
					renderbuffer.push_back(m_renderqueueshaders[shaderindex].SortedInstances[i].Item);
				}
			}
		}

		// Clear the sorted instance vector ready for the next frame
		m_renderqueueshaders[shaderindex].SortedInstances.clear();
	}
}

// Renders all objects in the specified system, based on simulation state and visibility testing
void CoreEngine::RenderAllSystemObjects(SpaceSystem *system)
{
	iSpaceObject *object;

	// Parameter check
	if (!system) return;

	// Iterate through all objects in the system object collection
	std::vector<iSpaceObject*>::iterator it_end = system->Objects.end();
	for (std::vector<iSpaceObject*>::iterator it = system->Objects.begin(); it != it_end; ++it)
	{
		// Get a reference to the object and make sure it is valid
		object = (*it); if (!object) continue;

		// Take different action based on object simulation state
		if (object->SimulationState() == iObject::ObjectSimulationState::FullSimulation)
		{
			// Pass to different methods depending on the type of object
			switch (object->GetObjectType())
			{
				// Object types with specialised rendering methods
				case iObject::ObjectType::SimpleShipObject:
					RenderSimpleShip((SimpleShip*)object);				break;
				case iObject::ComplexShipObject:
					RenderComplexShip((ComplexShip*)object, false);		break;

				// Basic object types are directly pushed to the render queue using default shader parameters
				case iObject::ProjectileObject:
					if (object && object->GetModel())
						SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, object->GetModel(), object->GetWorldMatrix());
					break;
			}
		}
	}

	// Now perform any post-processing.  Render all actors that have been queued up during rendering inside space environments, 
	// so that we only need to change actor-specific engine states once per frame (instead of every time a model is rendered in the collection above)
	ProcessQueuedActorRendering();
}

// RenderComplexShip: Renders a complex ship to the space environment.  Visibility & rendering is determined by section for efficiency
RJ_PROFILED(void CoreEngine::RenderComplexShip, ComplexShip *ship, bool renderinterior)
{
	bool render = false;								// Stores whether we should render the section currently being processed
	bool shiprendered = false;							// Stores whether any part of the ship was rendered this frame
	ComplexShipSection *sec;

	// Make sure we have valid parameters
	if (!ship) return;

	// Check whether this ship is, or contains, a simulation hub
	bool is_hub = (ship->IsSimulationHub() || ship->ContainsSimulationHubs());

	// Perform a visibility test on each section and render it if it falls within the viewing frustrum
	// First get a reference to the collection of ship sections, then iterate over it & test visibility one section at a time
	ComplexShip::ComplexShipSectionCollection::const_iterator it_end = ship->GetSections()->end();
	for (ComplexShip::ComplexShipSectionCollection::const_iterator it = ship->GetSections()->begin(); it != it_end; ++it)
	{
		// Get a reference to this ship section
		sec = (*it); if (!sec) continue;

		// If this ship is or contains a hub then we automatically render it.  Otherwise, perform a 
		// visibility test to see whether the section should be rendered.
		if (is_hub || m_frustrum->TestObjectVisibility(sec))
		{
			// Otherwise, perform an actual visibility test on the section to see if it should be rendered
			render = shiprendered = true;
		}
		else
		{
			render = false;
		}

		// If any bounding object passed the render test then render this ship section now
		if (render) RenderComplexShipSection(ship, sec);
	}

	// Pass control to the environment-rendering logic to render all visible objects within the environment, if applicable.
	// Criteria are that either (a)it was requested, (b) the ship is or contains a simulation hub, or (c) the flag is set 
	// that forces the interior to always be rendered.  In addition, we must have actually rendered some part of the ship.
	if (shiprendered && (renderinterior || is_hub || ship->InteriorShouldAlwaysBeRendered()))
	{
		RenderObjectEnvironment(ship);
	}

	// Increment the complex ship render count if any of its sections were rendered this frame
	if (shiprendered) ++m_renderinfo.ComplexShipRenderCount;
}
	
// Render a complex ship section to the space environment, as part of the rendering of the complex ship itself
void CoreEngine::RenderComplexShipSection(ComplexShip *ship, ComplexShipSection *sec)
{
	// Make sure this is a valid section and that it has a model for rendering
	if (!sec || !sec->GetModel()) return;

	// Render the exterior of the ship, unless we have the relevant render flag set
	if (!m_renderflags[CoreEngine::RenderFlag::DisableHullRendering])
	{	
		// We are rendering this object, so call its pre-render update method
		sec->PerformRenderUpdate();

		// Add this ship to the render queue for the relevant shader, for either normal or z-sorted processing
		if (sec->Fade.AlphaIsActive())
		{
			// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
			m_instanceparams.x = sec->Fade.GetFadeAlpha();
			if (m_instanceparams.x < Game::C_EPSILON) return;

			SubmitForZSortedRendering(CoreEngine::RenderQueueShader::RM_LightFadeShader, sec->GetModel(), sec->GetWorldMatrix(), 
									  m_instanceparams, sec->GetPosition());
		}
		else
		{
			if (sec->Highlight.IsActive())
			{
				m_instanceparams = sec->Highlight.GetColour();
				SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightHighlightShader, sec->GetModel(), sec->GetWorldMatrix(), m_instanceparams);
			}
			else
			{
				SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, sec->GetModel(), sec->GetWorldMatrix());
			}
		}
	}

	// Increment the render count
	++m_renderinfo.ComplexShipSectionRenderCount;
}

// Method to render the interior of an object environment, including any tiles, 
RJ_PROFILED(void CoreEngine::RenderObjectEnvironment, iSpaceObjectEnvironment *environment)
{
	// Parameter check
	if (!environment) return;

	// Precalculate the environment (0,0,0) point in world space, from which we can calculate element/tile/object positions.  
	D3DXVec3TransformCoord(&m_cache_zeropoint, &NULL_VECTOR, environment->GetZeroPointWorldMatrix());

	// Also calculate a set of effective 'basis' vectors, that indicate the change in world space 
	// required to move in +x, +y and +z directions
	D3DXVec3TransformCoordArray(m_cache_el_inc, sizeof(D3DXVECTOR3), m_cache_el_inc_base, sizeof(D3DXVECTOR3), environment->GetOrientationMatrix(), 3U);
	
	// Clear the temporary vectors used to prevent potential multiple rendering of objects that span >1 element
	m_tmp_renderedtiles.clear();
	m_tmp_renderedobjects.clear();
	m_tmp_renderedterrain.clear();

	// Begin the recursive environment traversal with one full, environment-sized element area
	RenderObjectEnvironmentSector(environment, NULL_INTVECTOR3, environment->GetElementSize());
}

// Recursively analsyses and renders a sector of the environment.  Performs binary splitting to efficiently test visibility.
// Only called internally so no parameter checks are performed, for efficiency.
void CoreEngine::RenderObjectEnvironmentSector(iSpaceObjectEnvironment *environment, const INTVECTOR3 & start, const INTVECTOR3 & size)
{
	// Make sure the size parameter is valid.  If any dimensions is zero, we know that this is an unneccessary zero-size area 
	// generated when we subdivided by a dimension that had a size of 1.  We can therefore abandon it here since the elements will
	// be covered in one of the other subdivisions (that will be exactly equal)
	if (size.x <= 0 || size.y <= 0 || size.z <= 0) return;

	// Determine the centre point of this element range.  Swap Y and Z coordinates since we are moving from environment to world space
	D3DXVECTOR3 centre = m_cache_zeropoint + (((float)start.x + ((float)size.x * 0.5f)) * m_cache_el_inc[0]) +
											 (((float)start.z + ((float)size.z * 0.5f)) * m_cache_el_inc[1]) +
											 (((float)start.y + ((float)size.y * 0.5f)) * m_cache_el_inc[2]);

	/*D3DXVECTOR3 centre = D3DXVECTOR3(	m_cache_zeropoint.x + (((float)start.x + ((float)size.x * 0.5f)) * m_cache_el_inc.x),
										m_cache_zeropoint.y + (((float)start.z + ((float)size.z * 0.5f)) * m_cache_el_inc.y),
										m_cache_zeropoint.z + (((float)start.y + ((float)size.y * 0.5f)) * m_cache_el_inc.z) );*/

	// Determine the maximum dimension, and then perform a lookup of the bounding sphere radius that completely encloses it
	float radius = GetElementBoundingSphereRadius(max(max(size.x, size.y), size.z));

	// Test visibility of this element range via a simple bounding sphere test.  
	// TOOD: Later, may be worth switching to bounding box test to more strictly exclude content (at the slight expense of test complexity)
	if (m_frustrum->CheckSphere(&centre, radius) == false) return;

	// This area is within the viewing frustum.  If the range is currently greater than 1x1x1 element then we want
	// to subdivide and move recursively downwards
	if (size.x != 1 || size.y != 1 || size.z != 1)
	{
		// Determine the midpoint, around which we will split the recursive subdivision.  Use integer division where 
		// the size in a dimension is even to avoid the potential for floating-point errors; we DO NOT want to cover 
		// the same element twice due to a fp precision issue or we may render its contents twice.
		INTVECTOR3 midpoint = INTVECTOR3(	((size.x & 1) ? (int)ceilf((float)size.x * 0.5f) : size.x / 2),
											((size.y & 1) ? (int)ceilf((float)size.y * 0.5f) : size.y / 2),
											((size.z & 1) ? (int)ceilf((float)size.z * 0.5f) : size.z / 2)  );

		// Precalculate some element offsets that are used multiple times in the following function calls
		INTVECTOR3 start_plus_midpoint = (start + midpoint);
		INTVECTOR3 size_less_midpoint = (size - midpoint); 

		// Recursively move into each sub-sector.  Attempt all eight; once any dimension reaches 1 we will start 
		// generating subdivisions with at least one dimension of size zero.  This is fine; we abandon them at the 
		// start of the method.
		RenderObjectEnvironmentSector(environment, start, midpoint);																								// 0,0,0
		RenderObjectEnvironmentSector(environment, INTVECTOR3(start_plus_midpoint.x, start.y, start.z), INTVECTOR3(size_less_midpoint.x, midpoint.y, midpoint.z));	// x,0,0
		RenderObjectEnvironmentSector(environment, INTVECTOR3(start.x, start_plus_midpoint.y, start.z), INTVECTOR3(midpoint.x, size_less_midpoint.y, midpoint.z));	// 0,y,0
		RenderObjectEnvironmentSector(environment, INTVECTOR3(start_plus_midpoint.x, start_plus_midpoint.y, start.z),												// x,y,0
												   INTVECTOR3(size_less_midpoint.x, size_less_midpoint.y, midpoint.z));
		RenderObjectEnvironmentSector(environment, INTVECTOR3(start.x, start.y, start_plus_midpoint.z), INTVECTOR3(midpoint.x, midpoint.y, size_less_midpoint.z));	// 0,0,z
		RenderObjectEnvironmentSector(environment, INTVECTOR3(start_plus_midpoint.x, start.y, start_plus_midpoint.z),												// x,0,z
												   INTVECTOR3(size_less_midpoint.x, midpoint.y, size_less_midpoint.z));
		RenderObjectEnvironmentSector(environment, INTVECTOR3(start.x, start_plus_midpoint.y, start_plus_midpoint.z),												// 0,y,z
												   INTVECTOR3(midpoint.x, size_less_midpoint.y, size_less_midpoint.z));
		RenderObjectEnvironmentSector(environment, start_plus_midpoint, size_less_midpoint);																		// x,y,z
	}
	else
	{
		// Otherwise, this is a single element that we have determined is visible.  We therefore want to render everything in it now
		RenderObjectEnvironmentSectorContents(environment, start);
	}

}


// Renders the contents of an element, including all linked tiles, objects & terrain.  Updates the temporary
// render lists to ensure an item that spans multiple elements is not rendered more than once
// Only called internally so no parameter checks are performed, for efficiency.
void CoreEngine::RenderObjectEnvironmentSectorContents(iSpaceObjectEnvironment *environment, const INTVECTOR3 & element)
{
	// Get a reference to this element, and make sure it is valid
	ComplexShipElement *el = environment->GetElement(element);
	if (!el) return;

	// We will only render this element if it is being fully-simulated
	if (el->GetSimulationState() != iObject::ObjectSimulationState::FullSimulation) return;

	// Iterate through the collection of tiles linked to this element
	if (el->HasTiles())
	{
		ComplexShipTile *tile;
		iContainsComplexShipTiles::ConstTileIterator it_end = el->GetTileIteratorEnd();
		for (iContainsComplexShipTiles::ConstTileIterator it = el->GetTileIteratorStart(); it != it_end; ++it)
		{
			// Make sure the item is valid
			tile = (*it); if (!tile) continue;

			// The InsertIntoSortedVectorIfNotPresent method will return 'true' if the item did not exist and was added, 
			// which in this case means we want to render it.  It it returns false the item has already been rendered 
			// (from its link to another element) so can therefore skip it.  We only need to check this if the item
			// spans multiple elements; if it doesn't, there is no way it could be rendered twice
			if (tile->SpansMultipleElements() && 
				InsertIntoSortedVectorIfNotPresent<Game::ID_TYPE>(m_tmp_renderedtiles, tile->GetID()) == false)
					continue;

			// Either the tile exists only in this element, or it spans multiple but hasn't been rendered yet.
			// We can therefore go ahead and render the tile relative to its parent ship section
			RenderComplexShipTile(tile, environment);
		}
	}

	// Iterate through the collection of active objects in (or partially in) this element
	if (el->HasObjects()) 
	{
		iEnvironmentObject *object;
		std::vector<iEnvironmentObject*>::iterator it2_end = el->Objects.end();
		for (std::vector<iEnvironmentObject*>::iterator it2 = el->Objects.begin(); it2 != it2_end; ++it2)
		{
			// Get a reference to the object and make sure it is valid
			object = (*it2); if (!object) continue;

			// (We no longer need to test the object simulation state, since we know this element is being
			// fully-simulated, and everything within it will inherit that simulation state)

			// The InsertIntoSortedVectorIfNotPresent method will return 'true' if the item did not exist and was added, 
			// which in this case means we want to render it.  It it returns false the item has already been rendered 
			// (from its link to another element) so can therefore skip it.  We only need to check this if the item
			// spans multiple elements; if it doesn't, there is no way it could be rendered twice
			if (object->SpansMultipleElements() &&
				InsertIntoSortedVectorIfNotPresent<Game::ID_TYPE>(m_tmp_renderedobjects, object->GetID()) == false)
					continue;

			// Pass to different methods depending on the type of object
			switch (object->GetObjectType())
			{
				case iObject::ActorObject:
					QueueActorRendering((Actor*)object);				break;
			}
		}
	}

	// Iterate through the collection of terrain objects in (or partially in) this element
	if (el->HasTerrain()) 
	{
		StaticTerrain *terrain;
		std::vector<StaticTerrain*>::iterator it3_end = el->TerrainObjects.end();
		for (std::vector<StaticTerrain*>::iterator it3 = el->TerrainObjects.begin(); it3 != it3_end; ++it3)
		{
			// Get a reference to the object and make sure it is valid, has a model etc.
			terrain = (*it3);
			if (!terrain || !terrain->GetDefinition() || !terrain->GetDefinition()->GetModel()) continue;

			// (We no longer need to test the object simulation state, since we know this element is being
			// fully-simulated, and everything within it will inherit that simulation state)

			// The InsertIntoSortedVectorIfNotPresent method will return 'true' if the item did not exist and was added, 
			// which in this case means we want to render it.  It it returns false the item has already been rendered 
			// (from its link to another element) so can therefore skip it.  We only need to check this if the item
			// spans multiple elements; if it doesn't, there is no way it could be rendered twice
			if (terrain->SpansMultipleElements() &&
				InsertIntoSortedVectorIfNotPresent<Game::ID_TYPE>(m_tmp_renderedterrain, terrain->GetID()) == false)
					continue;

			// We want to render this terrain object; compose the terrain world matrix with its parent environment world matrix to get the final transform
			D3DXMatrixMultiply(&m_tmp_matrix, terrain->GetWorldMatrix(), environment->GetZeroPointWorldMatrix());

			// Submit directly to the rendering pipeline.  Terrain objects are (currently) just a static model
			SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, terrain->GetDefinition()->GetModel(), &m_tmp_matrix);
			++m_renderinfo.TerrainRenderCount;
		}
	}
}


// Render a complex ship tile to the space environment, relative to its parent ship object
void CoreEngine::RenderComplexShipTile(ComplexShipTile *tile, iSpaceObjectEnvironment *environment)
{
	// Parameter check
	if (!tile) return;

	// Calculate the world matrix for this tile based on its parent and the tile offset matrix
	D3DXMATRIX wm;
	const D3DXMATRIX *mship = environment->GetZeroPointWorldMatrix();
	const D3DXMATRIX *mtile = tile->GetWorldMatrix();

	// Calculate the absolute world matrix for this tile as (WM = Child * Parent)
	D3DXMatrixMultiply(&wm, mtile, mship); 

	// We are rendering this object, so call its pre-render update method
	tile->PerformRenderUpdate();

	// Handle differently depending on whether this is a single or compound tile
	if (!tile->HasCompoundModel())
	{
		// Add the single tile model to the render queue, for either normal or z-sorted processing
		if (tile->GetModel()) 
		{
			if (tile->Fade.AlphaIsActive())
			{
				// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
				m_instanceparams.x = tile->Fade.GetFadeAlpha();
				if (m_instanceparams.x < Game::C_EPSILON) return;

				SubmitForZSortedRendering(	CoreEngine::RenderQueueShader::RM_LightFadeShader, tile->GetModel(), &wm, m_instanceparams, 
											D3DXVECTOR3(wm._41, wm._42, wm._43) );	// Position can be taken from trans. components of world matrix
			}
			else
			{
				if (tile->Highlight.IsActive())
				{
					m_instanceparams = tile->Highlight.GetColour();
					SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightHighlightShader, tile->GetModel(), &wm, m_instanceparams);
				}
				else
				{
					SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, tile->GetModel(), &wm);
				}
			}
		}
	}
	else
	{	
		D3DXMATRIX offset, modelwm; 

		// Determine whether any effects are active on the tile; these will need to be propogated across to constituent parts here
		bool fade = false;
		if (tile->Fade.AlphaIsActive()) { fade = true; m_instanceparams.x = tile->Fade.GetFadeAlpha(); }

		// Iterate through all models to be rendered
		ComplexShipTile::TileCompoundModelSet::TileModelCollection::const_iterator it_end = tile->GetCompoundModelSet()->Models.end();
		for (ComplexShipTile::TileCompoundModelSet::TileModelCollection::const_iterator it = tile->GetCompoundModelSet()->Models.begin(); it != it_end; ++it)
		{
			// Make sure this is a valid model
			if (!it->model) continue;
			
			// Apply a transformation of (ModelRotation * CurrentWorldMatrix * ModelTranslation)
			D3DXMatrixTranslation(&offset, it->offset.x, it->offset.y, it->offset.z);
			D3DXMatrixMultiply(&modelwm, &it->rotmatrix, &wm);
			D3DXMatrixMultiply(&modelwm, &modelwm, &offset);

			// Submit the tile model for rendering using this adjusted world matrix
			if (fade)
			{
				SubmitForZSortedRendering(	CoreEngine::RenderQueueShader::RM_LightFadeShader, it->model, &modelwm, m_instanceparams, 
											D3DXVECTOR3(modelwm._41, modelwm._42, modelwm._43) );	// Take pos from the trans components of the world matrix
			}
			else
			{
				SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, it->model, &modelwm);
			}

		}
	}

	// Increment the render count
	++m_renderinfo.ComplexShipTileRenderCount;
}

// Render a simple ship to the space environment
#ifndef RJ_PROFILER_ACTIVE	
	void CoreEngine::RenderSimpleShip(SimpleShip *s)
#else	
	void CoreEngine::RenderSimpleShip_Profiled(SimpleShip *s)
#endif
{
	// Make sure the ship exists and has a model we can render
	if (!s || !s->GetModel()) return;

	// Test whether this ship is within the viewing frustrum; if not, no need to render it
	if (!m_frustrum->TestObjectVisibility(s)) return;

	// We are rendering this object, so call its pre-render update method
	s->PerformRenderUpdate();

	// Add this ship to the render queue for the relevant shader
	if (s->Fade.AlphaIsActive())
	{
		// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
		m_instanceparams.x = s->Fade.GetFadeAlpha();
		if (m_instanceparams.x < Game::C_EPSILON) return;

		SubmitForZSortedRendering(CoreEngine::RenderQueueShader::RM_LightFadeShader, s->GetModel(), s->GetWorldMatrix(), m_instanceparams, s->GetPosition());
	}
	else
	{
		if (s->Highlight.IsActive())
		{
			m_instanceparams = s->Highlight.GetColour();
			SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightHighlightShader, s->GetModel(), s->GetWorldMatrix(), m_instanceparams);
		}
		else
		{
			SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, s->GetModel(), s->GetWorldMatrix());
		}
	}

	// Increment the render count
	++m_renderinfo.ShipRenderCount;
}

// Rendering method for skinned models
void CoreEngine::RenderSkinnedModelInstance(SkinnedModelInstance &model)
{

}

RJ_PROFILED(void CoreEngine::RenderImmediateRegion, void)
{
	// Prepare all region particles, calculate vertex data and promote all to the buffer ready for shader rendering
	D::Regions::Immediate->Render(m_D3D->GetDeviceContext(), &r_view);

	// Get a handle to the world matrix (required?  correct?  will this not be the last matrix set in previous methods?  and will it actually be used?)
	D3DXMATRIX world;
	m_D3D->GetWorldMatrix(world);
	D3DXMatrixIdentity(&world);

	// Enable alpha blending before rendering any particles
	m_D3D->SetAlphaBlendModeEnabled();
	m_D3D->DisableZBufferWriting();

	// Render dust particles using the particle shader
	m_particleshader->Render( r_devicecontext, D::Regions::Immediate->GetActiveParticleCount() * 6, 
							  world, r_view, r_projection, D::Regions::Immediate->GetParticleTextureResource() );

	// Disable alpha blending after rendering the particles
	// TODO: Move to somewhere more global once we have further rendering that requires alpha effects.  Avoid multiple on/off switches
	m_D3D->SetAlphaBlendModeDisabled();	
	m_D3D->EnableZBuffer();
}

RJ_PROFILED(void CoreEngine::RenderSystemRegion, void)
{
	// We will simply use ID for the world matrix as it is not relevant here
	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	world._41 = r_invview._41; world._42 = r_invview._42; world._43 = r_invview._43;

	// Render system backdrop using the standard texture shader, if there is a backdrop to render
	if (D::Regions::System->HasSystemBackdrop())
	{
		// Disable backface culling, to render inside the skybox, and also disable the z-buffer 
		// so that all subsequent objects are drawn over the top
		m_D3D->DisableRasteriserCulling();
		m_D3D->DisableZBuffer();

		// Render all backdrop data to vertex buffers
		D::Regions::System->Render(m_D3D->GetDeviceContext());

		// Render the buffers using the standard texture shader
		m_texcubeshader->Render( r_devicecontext, D::Regions::System->GetBackdropSkybox()->GetIndexCount(), 
								 world, r_view, r_projection,
								 D::Regions::System->GetBackdropTextureResource() );

		// Re-enable culling and the z-buffer after rendering the skybox
		m_D3D->EnableRasteriserCulling();
		m_D3D->EnableZBuffer();
	}
}

RJ_PROFILED(void CoreEngine::RenderUserInterface, void)
{
	// Enable alpha-blending, and disable the z-buffer, in advance of any 2D UI rendering
	m_D3D->SetAlphaBlendModeEnabled();
//	m_D3D->DisableZBuffer();

	// Call the UI controller rendering function
	D::UI->Render();

	// Render all 2D components to the screen
	m_render2d->Render();

	// Render the text strings to the interface
	m_textmanager->Render(ID_MATRIX, r_orthographic);

	// Now disable alpha blending, and re-enable the z-buffer, to return to normal rendering
	m_D3D->SetAlphaBlendModeDisabled();
//	m_D3D->EnableZBuffer();
}

RJ_PROFILED(void CoreEngine::RenderEffects, void)
{
	// Enable alpha blending before rendering any effects
	m_D3D->SetAlphaBlendModeEnabled();

	// Update the effect manager frame timer to ensure effects are rendered consistently regardless of FPS
	m_effectmanager->BeginEffectRendering();
//TODO: REMOVE
	// Render a test fire effect
	if (false) {
		D3DXMATRIX world, tmpT, tmpR, tmpS;
		D3DXQUATERNION *tmpQ;
		D3DXMatrixTranslation(&tmpT, 0,0,125);
		tmpQ = m_camera->GetDecomposedViewRotation();
		//D3DXQuaternionIdentity(&tmpQ);
		D3DXMatrixRotationQuaternion(&tmpR, tmpQ);
		D3DXMatrixInverse(&tmpR, 0, &tmpR);
		D3DXMatrixScaling(&tmpS, 15.0f, 15.0f, 15.0f);
		D3DXMatrixMultiply(&tmpS, &tmpR, &tmpS);
		D3DXMatrixMultiply(&world, &tmpS, &tmpT);
		m_effectmanager->RenderFireEffect(0, r_devicecontext, world, r_view, r_projection);
	}

	// Disable alpha blending after all effects are rendered
	m_D3D->SetAlphaBlendModeDisabled();
}

RJ_PROFILED(void CoreEngine::RenderParticleEmitters, void)
{
	// Pass control to the particle engine to perform all required rendering
	m_particleengine->Render(r_view, r_projection, m_D3D, m_camera);
}

// Renders all actors that have been queued up for rendering
RJ_PROFILED(void CoreEngine::ProcessQueuedActorRendering, void)
{
	// Quit immediately if there are no queued actors to render
	if (m_queuedactors.empty()) return;

	// Disable rasteriser culling to prevent loss of intermediate faces in skinned models; we only have to do this once before rendering all actors
	m_D3D->DisableRasteriserCulling();

	// Get parameters that are used for every actor being rendered (TODO: this can be done once before rendering all actors, to improve efficiency)
	XMFLOAT3 campos = (XMFLOAT3)m_camera->GetPosition();
	XMFLOAT4X4 view = (XMFLOAT4X4)r_view;
	XMFLOAT4X4 proj = (XMFLOAT4X4)r_projection;

	// Iterate through the vector of queued actors
	std::vector<Actor*>::iterator it_end = m_queuedactors.end();
	for (std::vector<Actor*>::iterator it = m_queuedactors.begin(); it != it_end; ++it)
	{
		// Calculate new transforms for the bones & mesh of this actor
		(*it)->UpdateForRendering(Game::TimeFactor);

		// Render using the skinned model shader
		m_skinnedshader->Render(r_devicecontext, (*it)->GetModel(), campos, view, proj);
	}

	// Enable rasteriser culling again once all actors have been rendered
	m_D3D->EnableRasteriserCulling();

	// Increase the count of actors we processed; we can count every item that was in the queue, since they 
	// were validated before being placed in the queue so should all render
	m_renderinfo.ActorRenderCount += m_queuedactors.size();

	// Reset the queue now that it has been processed, so we are ready for adding new items in the next frame
	m_queuedactors.clear();
}

void CoreEngine::ClearRenderingQueue(void)
{
	RM_ShaderModelInstanceMap::iterator smi, smi_end;
	RM_ModelInstanceMap::iterator mi, mi_end;

	// Iterate through each shader in the render queue
	for (int i = 0; i < CoreEngine::RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; i++)
	{
		// Iterate through each model queued for rendering by this shader
		mi_end = m_renderqueue[i].end();
		for (mi = m_renderqueue[i].begin(); mi != mi_end; ++mi)
		{
			// Clear all instance data from the associated vector
			mi->second.clear();
		}
	}
}

// Performs rendering of debug/special data
void CoreEngine::RenderDebugData(void)
{
	if (m_renderflags[CoreEngine::RenderFlag::RenderTree]) DebugRenderSpatialPartitioningTree();
	if (m_renderflags[CoreEngine::RenderFlag::RenderOBBs]) DebugRenderSpaceCollisionBoxes();
	if (m_renderflags[CoreEngine::RenderFlag::RenderTerrainBoxes]) DebugRenderEnvironmentCollisionBoxes();

}

void CoreEngine::DebugRenderSpatialPartitioningTree(void)
{
	if (Game::CurrentPlayer && Game::CurrentPlayer->GetSystem() && Game::CurrentPlayer->GetSystem()->SpatialPartitioningTree)
		Game::CurrentPlayer->GetSystem()->SpatialPartitioningTree->DebugRender(true);
}

void CoreEngine::DebugRenderSpaceCollisionBoxes(void)
{
	iSpaceObject *object;
	std::vector<iSpaceObject*> objects; int count = 0;
	D3DXVECTOR3 pos; float radius; bool invalidated;

	// Find all active space objects around the player; take a different approach depending on whether the player is in a ship or on foot
	if (Game::CurrentPlayer->GetState() == Player::StateType::OnFoot)
	{
		// Player is on foot, so use a proximity test to the object currently considered their parent environment
		if (Game::CurrentPlayer->GetParentEnvironment() == NULL) return;
		count = 1 + Game::ObjectManager.GetAllObjectsWithinDistance(Game::CurrentPlayer->GetParentEnvironment(), 10000.0f, objects,
																   (SimulationObjectManager::ObjectSearchOptions::OnlyCollidingObjects));

		// Also include the parent ship environmment (hence why we +1 to the count above)
		objects.push_back((iSpaceObject*)Game::CurrentPlayer->GetParentEnvironment());
	}
	else
	{
		// Player is in a spaceobject ship, so use the proximity test on their ship
		if (Game::CurrentPlayer->GetPlayerShip() == NULL) return;
		count = 1 + Game::ObjectManager.GetAllObjectsWithinDistance(Game::CurrentPlayer->GetPlayerShip(), 10000.0f, objects, 
																   (SimulationObjectManager::ObjectSearchOptions::OnlyCollidingObjects));

		// Also include the player ship (hence why we +1 to the count above)
		objects.push_back((iSpaceObject*)Game::CurrentPlayer->GetPlayerShip());
	}

	// Iterate through the active objects
	std::vector<iSpaceObject*>::iterator it_end = objects.end();
	for (std::vector<iSpaceObject*>::iterator it = objects.begin(); it != it_end; ++it)
	{
		object = (*it);
		pos = object->GetPosition();
		radius = object->GetCollisionSphereRadius();

		// Render the oriented bounding box(es) used for narrowphase collision detection, if applicable for this object
		// Also force an immediate recalculation of any invalidated OBBs so that they can be debug-rendered
		if (object->GetCollisionMode() == Game::CollisionMode::FullCollision)
		{
			// Test whether the OBB is invalidated and therefore needs to be recalculated
			invalidated = object->CollisionOBB.IsInvalidated();
			if (invalidated) object->CollisionOBB.UpdateFromObject(*object);

			// Render the OBB
			Game::Engine->GetOverlayRenderer()->RenderOBB(object->CollisionOBB, true, 
				(invalidated ? OverlayRenderer::RenderColour::RC_LightBlue : OverlayRenderer::RenderColour::RC_Green), 0.1f);
		}
	}
}

void CoreEngine::DebugRenderEnvironmentCollisionBoxes(void)
{
	// Parameter check
	if (m_debug_renderenvboxes == 0 || Game::Objects.count(m_debug_renderenvboxes) == 0) return;
	D3DXVECTOR3 v[8]; iEnvironmentObject *a_obj; StaticTerrain *t_obj;

	// Get a reference to the environment object
	iSpaceObjectEnvironment *parent = (iSpaceObjectEnvironment*)Game::Objects[m_debug_renderenvboxes];
	if (!parent) return;

	// Iterate through all active objects within this parent environment
	std::vector<iEnvironmentObject*>::iterator a_it_end = parent->Objects.end();
	for (std::vector<iEnvironmentObject*>::iterator a_it = parent->Objects.begin(); a_it != a_it_end; ++a_it)
	{
		a_obj = (*a_it);
		if (a_obj)
		{
			// Determine the location of all vertices for each bounding volume and render using the overlay renderer
			a_obj->CollisionOBB.DetermineVertices(v);
			Game::Engine->GetOverlayRenderer()->RenderCuboid(v, OverlayRenderer::RenderColour::RC_LightBlue, 0.1f);
		}
	}

	// Iterate through all terrain objects within this parent environment
	std::vector<StaticTerrain*>::iterator t_it_end = parent->TerrainObjects.end();
	for (std::vector<StaticTerrain*>::iterator t_it = parent->TerrainObjects.begin(); t_it != t_it_end; ++t_it)
	{
		t_obj = (*t_it);
		if (t_obj)
		{
			// Determine the location of all vertices for each bounding volume and render using the overlay renderer
			t_obj->DetermineCollisionBoxVertices(parent, v);
			Game::Engine->GetOverlayRenderer()->RenderCuboid(v, OverlayRenderer::RenderColour::RC_Red, 0.1f);
		}
	}
}


// Activate or deactivate a particular stage of the rendering cycle.  Changing the 'All' stage will overwrite all stage values
void CoreEngine::SetRenderStageState(CoreEngine::RenderStage stage, bool active)
{
	// Make sure the specified stage is valid
	if ((int)stage >= 0 && (int)stage < (int)CoreEngine::RenderStage::Render_STAGECOUNT)
	{
		// Change the state of this stage
		m_renderstages[(int)stage] = active;

		// If this is the "All" stage, update all other stages directly
		if (stage == CoreEngine::RenderStage::Render_ALL)
		{
			for (int i = 0; i < (int)CoreEngine::RenderStage::Render_STAGECOUNT; ++i)
				m_renderstages[i] = active;
		}
	}
}

// Virtual inherited method to accept a command from the console
bool CoreEngine::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "render_tree")
	{
		if (!Game::CurrentPlayer || !Game::CurrentPlayer->GetSystem() || !Game::CurrentPlayer->GetSystem()->SpatialPartitioningTree)
			{ command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::NoSpatialPartitioningTreeToRender, 
				"No tree to render"); return true; }

		bool b = (command.Parameter(0) == "1");
		SetRenderFlag(CoreEngine::RenderFlag::RenderTree, b);
		command.SetSuccessOutput(concat((b ? "Enabling" : "Disabling"))(" render of spatial partitioning tree").str());
		return true;
	}
	else if (command.InputCommand == "hull_render")
	{
		bool b = !(command.Parameter(0) == "1");
		SetRenderFlag(CoreEngine::RenderFlag::DisableHullRendering, b);
		command.SetSuccessOutput(concat((b ? "Disabling" : "Enabling"))(" rendering of ship hulls").str()); return true;
	}
	else if (command.InputCommand == "render_obb")
	{
		bool b = (command.Parameter(0) == "1");
		SetRenderFlag(CoreEngine::RenderFlag::RenderOBBs, b);
		command.SetSuccessOutput(concat((b ? "Enabling" : "Disabling"))(" rendering of object OBBs").str()); return true;
	}
	else if (command.InputCommand == "render_terrainboxes")
	{
		if (command.Parameter(1) == "1")
		{
			iObject *env = Game::FindObjectInGlobalRegister(command.Parameter(0));
			if (!env || !env->IsEnvironment())
				{ command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
						concat("Invalid object \"")(command.Parameter(0))("\" specified").str()); return true; }

			SetRenderFlag(CoreEngine::RenderFlag::RenderTerrainBoxes, true);
			SetDebugTerrainRenderEnvironment(env->GetID());
			command.SetSuccessOutput("Enabling render of environment object collision volumes");
		}
		else
		{
			SetRenderFlag(CoreEngine::RenderFlag::RenderTerrainBoxes, false);
			m_debug_renderenvboxes = 0;
			command.SetSuccessOutput("Disabling render of environment collision volumes");
		}
		return true;
	}

	// We did not recognise the command
	return false;
}

// Resets all render statistics ready for the next frame
void CoreEngine::ResetRenderInfo(void)
{
	memset(&m_renderinfo, 0, sizeof(EngineRenderInfoData));
}

// Default destructor
CoreEngine::~CoreEngine(void)
{
}

