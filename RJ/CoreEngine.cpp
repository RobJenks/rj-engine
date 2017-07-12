#include "DX11_Compatibility.h"		// Should be included first when performing compatibility checks
#include "DX11_Core.h" 

#include "ErrorCodes.h"
#include "GlobalFlags.h"
#include "Logging.h"
#include "D3DMain.h"
#include "RJMain.h"
#include "Profiler.h"
#include "FrameProfiler.h"
#include "Timers.h"
#include "CameraClass.h"
#include "InputLayoutDesc.h"
#include "LightingManagerObject.h"
#include "LightShader.h"
#include "LightFadeShader.h"
#include "LightHighlightShader.h"
#include "LightHighlightFadeShader.h"
#include "LightFlatHighlightFadeShader.h"
#include "ParticleShader.h"
#include "TextureShader.h"
#include "TexcubeShader.h"
#include "FireShader.h"
#include "Light.h"
#include "ViewFrustrum.h"
#include "BoundingObject.h"
#include "FontShader.h"
#include "AudioManager.h"
#include "TextManager.h"
#include "Fonts.h"
#include "EffectManager.h"
#include "ParticleEngine.h"
#include "Render2DManager.h"
#include "SkinnedNormalMapShader.h"
#include "VolLineShader.h"
#include "OverlayRenderer.h"
#include "BasicColourDefinition.h"

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
#include "GameObjects.h"
#include "ObjectSearch.h"
#include "GameUniverse.h"
#include "SpaceSystem.h"
#include "ImmediateRegion.h"
#include "SystemRegion.h"
#include "UserInterface.h"
#include "SkinnedModel.h"
#include "ArticulatedModel.h"
#include "TurretController.h"
#include "BasicProjectileSet.h"
#include "BasicProjectileDefinition.h"
#include "StaticTerrainDefinition.h"
#include "Actor.h"
#include "GameConsoleCommand.h"
#include "VolumetricLine.h"
#include "EnvironmentTree.h"
#include "NavNetwork.h"
#include "NavNode.h"
#include "SentenceType.h"
#include <tchar.h>
#include <unordered_map>

#include "CoreEngine.h"

using namespace std;


// Default constructor
CoreEngine::CoreEngine(void)
	:
	m_rq_optimiser(m_renderqueue)
{
	// Reset all component pointers to NULL, in advance of initialisation
	m_D3D = NULL;
	m_camera = NULL;
	m_lightshader = NULL;
	m_lightfadeshader = NULL;
	m_lighthighlightshader = NULL;
	m_lighthighlightfadeshader = NULL;
	m_particleshader = NULL;
	m_textureshader = NULL;
	m_texcubeshader = NULL;
	m_frustrum = NULL;
	m_textmanager = NULL;
	m_fontshader = NULL;
	m_fireshader = NULL;
	m_skinnedshader = NULL;
	m_vollineshader = NULL;
	m_effectmanager = NULL;
	m_particleengine = NULL;
	m_render2d = NULL;
	m_overlayrenderer = NULL;
	m_instancebuffer = NULL;
	m_debug_renderenvboxes = m_debug_renderenvtree = 0;
	m_debug_renderobjid_object = 0;
	m_debug_renderobjid_distance = 1000.0f;
	m_debug_terrain_render_mode = DebugTerrainRenderMode::Normal;
	m_current_topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	// Set default values for game engine parameters
	m_hwnd = NULL;
	m_vsync = false;

	// Initialise all render stage flags to true at startup
	m_renderstages = std::vector<bool>(CoreEngine::RenderStage::Render_STAGECOUNT, true);
	
	// Initialise all special render flags at startup
	m_renderflags = std::vector<bool>(CoreEngine::RenderFlag::_RFLAG_COUNT, false);

	// Set pre-populated parameter values for render-time efficiency
	m_current_modelbuffer = NULL;
	m_instanceparams = NULL_FLOAT4;

	// Initialise all key matrices to the identity
	r_view = r_projection = r_orthographic = r_invview = r_viewproj = r_invviewproj = m_projscreen 
		= r_viewprojscreen = r_invviewprojscreen = ID_MATRIX;
	r_view_f = r_projection_f = r_orthographic_f = r_invview_f = r_viewproj_f = r_invviewproj_f = ID_MATRIX_F;

	// Initialise all temporary/cache fields that are used for more efficient intermediate calculations
	m_cache_zeropoint = m_cache_el_inc[0].value = m_cache_el_inc[1].value = m_cache_el_inc[2].value = NULL_VECTOR;
	
	// Initialise the cached increment vectors, representing a +1 element movement in each dimension in turn
	m_cache_el_inc_base[0].value = XMVectorSet(Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f, 0.0f);
	m_cache_el_inc_base[1].value = XMVectorSet(0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	m_cache_el_inc_base[2].value = XMVectorSet(0.0f, 0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f);
}


Result CoreEngine::InitialiseGameEngine(HWND hwnd)
{ 
	Result res;

	// Initialise each component in turn; in case of failure, attempt to roll back anything possible and return an error
	Game::Log << "\n" << LOG_INFO << "Beginning initialisation of game engine\n";

	// Trigger the window resize event to ensure the engine has the latest window/screen parameters
	WindowResized();

	// Hide the system cursor
	SetSystemCursorVisibility(false);

	// Initialise all render flags to their default values
	res = InitialiseRenderFlags();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Render flags initialised\n";

	// Initialise DirectX math functions 
	res = InitialiseDirectXMath();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "DX Math initialised\n";

	// Initialise the Direct3D component
	res = InitialiseDirect3D(hwnd);
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Direct3D initialisation complete\n";

	// Initialise the camera component
	res = InitialiseCamera();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Camera initialised\n";

	// Initialise the lighting manager
	res = InitialiseLightingManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Lighting manager initialised\n";

	// Initialise shader support data
	res = InitialiseShaderSupport();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader support data initialised\n";

	// Initialise the light shader
	res = InitialiseLightShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Light] initialisation complete\n";

	// Initialise the light/fade shader
	res = InitialiseLightFadeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Light fade] initialisation complete\n";

	// Initialise the light/highlight shader
	res = InitialiseLightHighlightShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Light highlight] initialisation complete\n";

	// Initialise the light/highlight/fade shader
	res = InitialiseLightHighlightFadeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Light highlight fade] initialisation complete\n";

	// Initialise the light(flat)/highlight/fade shader
	res = InitialiseLightFlatHighlightFadeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Light flat highlight fade] initialisation complete\n";

	// Initialise the particle shader
	res = InitialiseParticleShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Particle] initialisation complete\n";

	// Initialise the texture shader
	res = InitialiseTextureShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Texture] initialisation complete\n";

	// Initialise the view frustrum
	res = InitialiseFrustrum();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "View frustum created\n";

	// Initialise the font shader
	res = InitialiseFontShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Font] initialisation complete\n";

	// Initialise the audio manager
	res = InitialiseAudioManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Audio manager initialisation complete\n";

	// Initialise the text rendering components
	res = InitialiseTextRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Text rendering initialised\n";

	// Initialise all game fonts
	res = InitialiseFonts();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Font initialisation complete\n";

	// Initialise the texcube shader
	res = InitialiseTexcubeShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Texcube] initialisation complete\n";

	// Initialise the fire shader
	res = InitialiseFireShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Fire] initialisation complete\n";

	// Initialise the effect manager
	res = InitialiseEffectManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Effect manager initialised\n";

	// Initialise the skinned normal map shader
	res = InitialiseSkinnedNormalMapShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Skinned normal map] initialisation complete\n";

	// Initialise the volumetric line shader
	res = InitialiseVolLineShader();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader [Volumetric line] initialisation complete\n";
	
	// Initialise the particle engine
	res = InitialiseParticleEngine();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Particle engine initialised\n" << LogManager::flush;

	// Initialise the 2D render manager
	res = Initialise2DRenderManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "2D render manager initialisation complete\n";

	// Initialise the overlay renderer
	res = InitialiseOverlayRenderer();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Overlay renderer initialised\n";
	
	// Initialise the render queue for geometry instancing & batching (dependent on initialisation of relevant shaders)
	res = InitialiseRenderQueue();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Render queue initialised for all shaders\n";

	// Initialise the components used for environment rendering
	res = InitialiseEnvironmentRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Environment rendering initialised\n";


	// If we succeed in all initialisation functions then return success now
	Game::Log << LOG_INFO << "All game engine initialisation completed successfully\n\n";
	return ErrorCodes::NoError;
}

void CoreEngine::ShutdownGameEngine()
{
	// Run the termination function for each component
	ShutdownLightShader();
	ShutdownLightFadeShader();
	ShutdownLightHighlightShader();
	ShutdownLightHighlightFadeShader();
	ShutdownLightFlatHighlightFadeShader();
	ShutdownParticleShader();
	ShutdownTextureShader();
	ShutdownCamera();
	ShutdownLightingManager();
	ShutdownShaderSupport();
	ShutdownFrustrum();
	ShutdownAudioManager();
	ShutdownTextRendering();
	ShutdownFontShader();
	ShutdownFonts();
	ShutdownTexcubeShader();
	ShutdownFireShader();
	ShutdownEffectManager();
	ShutdownSkinnedNormalMapShader();
	ShutdownVolLineShader();
	ShutdownParticleEngine();
	Shutdown2DRenderManager();
	ShutdownOverlayRenderer();
	ShutdownRenderQueue();
	ShutdownEnvironmentRendering();
	ShutdownTextureData();
	ShutdownDirect3D();
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
	m_D3D = new D3DMain();
	if ( !m_D3D ) return ErrorCodes::CannotCreateDirect3DDevice;

	// Initialise the Direct3D object.
	Result result = m_D3D->Initialise(Game::ScreenWidth, Game::ScreenHeight, m_vsync, m_hwnd, Game::FullScreen, 
									  SCREEN_DEPTH, SCREEN_NEAR);
	return result;
}

Result CoreEngine::InitialiseDirectXMath(void)
{
	// Test whether the client system supports SEE/SIMD instruction sets
	if (PlatformSupportsSSEInstructionSets())
	{
		Game::Log << LOG_INFO << "Platform supports SSE/SSE2 instruction sets\n";
	}
	else
	{
		Game::Log << LOG_WARN << "Warning: Platform does not support SSE/SSE2 instruction sets\n";
	}

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseRenderQueue(void)
{
	D3D11_BUFFER_DESC ibufdesc;
	D3D11_SUBRESOURCE_DATA ibufdata;

	// Create a new empty instance stream, for initialisation of the buffer
	RM_Instance *idata = (RM_Instance*)malloc(Game::C_INSTANCED_RENDER_LIMIT * sizeof(RM_Instance));
	if (!idata) return ErrorCodes::CouldNotAllocateMemoryForRenderQueue;
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
	m_instancedstride[0] = 0U;										// Stride[0] will be populated with the model-specific vertex size
	m_instancedstride[1] = sizeof(RM_Instance);
	m_instancedoffset[0] = 0; m_instancedoffset[1] = 0;

	// Initialise the render queue with a blank map for each shader in scope
	m_renderqueue = RM_RenderQueue(RenderQueueShader::RM_RENDERQUEUESHADERCOUNT);
	m_renderqueueshaders = RM_ShaderCollection(RenderQueueShader::RM_RENDERQUEUESHADERCOUNT);

	// Set the reference and parameters for each shader in turn
	m_renderqueueshaders[RenderQueueShader::RM_LightShader] = 
		RM_InstancedShaderDetails((iShader*)m_lightshader, false, D3DMain::AlphaBlendState::AlphaBlendDisabled, 
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_renderqueueshaders[RenderQueueShader::RM_LightHighlightShader] = 
		RM_InstancedShaderDetails((iShader*)m_lighthighlightshader, false, D3DMain::AlphaBlendState::AlphaBlendDisabled, 
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_renderqueueshaders[RenderQueueShader::RM_LightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lightfadeshader, true, D3DMain::AlphaBlendState::AlphaBlendEnabledNormal, 
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_renderqueueshaders[RenderQueueShader::RM_LightHighlightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lighthighlightfadeshader, true, D3DMain::AlphaBlendState::AlphaBlendEnabledNormal, 
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_renderqueueshaders[RenderQueueShader::RM_LightFlatHighlightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lightflathighlightfadeshader, true, D3DMain::AlphaBlendState::AlphaBlendEnabledNormal,
		D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
	m_renderqueueshaders[RenderQueueShader::RM_VolLineShader] =
		RM_InstancedShaderDetails((iShader*)m_vollineshader, true, D3DMain::AlphaBlendState::AlphaBlendEnabledNormal, 
		D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	// TODO: DEBUG: Remove variants on the light shader
	/*m_renderqueueshaders[RenderQueueShader::RM_LightHighlightShader] = m_renderqueueshaders[RenderQueueShader::RM_LightShader];
	m_renderqueueshaders[RenderQueueShader::RM_LightFadeShader] = m_renderqueueshaders[RenderQueueShader::RM_LightShader];
	m_renderqueueshaders[RenderQueueShader::RM_LightHighlightFadeShader] = m_renderqueueshaders[RenderQueueShader::RM_LightShader];*/

	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseCamera(void)
{
	// Attempt to create a new camera instance
	m_camera = new CameraClass();
	if ( !m_camera ) return ErrorCodes::CannotCreateCameraComponent;

	// Initialise the camera
	Result result = m_camera->Initialise();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Use the camera rendering functions to generate a temporary view matrix for all 2D text rendering (from a position of [0,0,-1])
	m_camera->CalculateViewMatrixFromPositionData(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), ID_QUATERNION, ID_MATRIX);
	m_camera->GetViewMatrix(m_baseviewmatrix);

	// Return success
	return ErrorCodes::NoError;

}

Result CoreEngine::InitialiseLightingManager(void)
{
	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseShaderSupport(void)
{
	// Initialise all standard vertex input layouts
	InputLayoutDesc::InitialiseStaticData();

	// Returns success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseLightShader(void)
{
	// Create the light shader object.
	m_lightshader = new LightShader();
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

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseLightFadeShader(void)
{
	// Create the light shader object.
	m_lightfadeshader = new LightFadeShader();
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

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}


Result CoreEngine::InitialiseLightHighlightShader(void)
{
	// Create the light shader object.
	m_lighthighlightshader = new LightHighlightShader();
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

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}


Result CoreEngine::InitialiseLightHighlightFadeShader(void)
{
	// Create the light shader object.
	m_lighthighlightfadeshader = new LightHighlightFadeShader();
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

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}


Result CoreEngine::InitialiseLightFlatHighlightFadeShader(void)
{
	// Create the light shader object.
	m_lightflathighlightfadeshader = new LightFlatHighlightFadeShader();
	if (!m_lightflathighlightfadeshader)
	{
		return ErrorCodes::CouldNotCreateLightFlatHighlightFadeShader;
	}

	// Initialise the light shader object.
	Result result = m_lightflathighlightfadeshader->Initialise(m_D3D->GetDevice(), m_hwnd);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success code if we have reached this point
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseParticleShader(void)
{
	// Create the particle shader object
	m_particleshader = new ParticleShader();
	if (!m_particleshader)
	{
		return ErrorCodes::CouldNotCreateParticleShader;
	}

	// Initialise the particle shader
	Result result = m_particleshader->Initialise(m_D3D->GetDevice(), m_hwnd);
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
	m_textureshader = new TextureShader();
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

Result CoreEngine::InitialiseFrustrum()
{
	// Attempt to create a new view frustrum
	m_frustrum = new ViewFrustrum();
	if (!m_frustrum) return ErrorCodes::CannotCreateViewFrustrum;
	
	// Run the initialisation function with viewport/projection data that can be precaulcated
	Result res = m_frustrum->Initialise(m_D3D->GetProjectionMatrix(), SCREEN_DEPTH, m_D3D->GetDisplayFOV(), m_D3D->GetDisplayAspectRatio());
	if (res != ErrorCodes::NoError) return res;	

	// Return success if the frustrum was created
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseFontShader(void)
{
	Result result;

	// Create the font shader object.
	m_fontshader = new FontShader();
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

Result CoreEngine::InitialiseAudioManager(void)
{
	Result result;

	// Create the text manager object
	m_audiomanager = new AudioManager();
	if (!m_audiomanager)
	{
		return ErrorCodes::CannotCreateAudioManager;
	}

	// Now attempt to initialise the audio manager object
	result = m_audiomanager->Initialise();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;

}

Result CoreEngine::InitialiseTextRendering(void)
{
	Result result;

	// Create the text manager object
	m_textmanager = new TextManager();
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
	result = m_textmanager->InitializeFont(	"Font_Basic1", 
											concat(D::DATA)("/Fonts/font_basic1.txt").str().c_str(), 
											concat(D::IMAGE_DATA)("/Fonts/font_basic1.dds").str().c_str(), 
											Game::Fonts::FONT_BASIC1);
	if (result != ErrorCodes::NoError) return result;

	// Return success when all fonts are loaded
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseTexcubeShader(void)
{
	// Create the texture shader object
	m_texcubeshader = new TexcubeShader();
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
	m_fireshader = new FireShader();
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
	m_effectmanager = new EffectManager();
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
	m_skinnedshader = new SkinnedNormalMapShader();
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

Result CoreEngine::InitialiseVolLineShader(void)
{
	// Initialise the static data used in volumetric line rendering
	Result result = VolLineShader::InitialiseStaticData(GetDevice());
	if (result != ErrorCodes::NoError) return result;

	// Create a new instance of the shader object
	m_vollineshader = new VolLineShader();
	if (!m_vollineshader)
	{
		return ErrorCodes::CannotCreateVolumetricLineShader;
	}

	// Now attempt to initialise the shader
	result = m_vollineshader->Initialise(m_D3D->GetDevice(), XMFLOAT2((float)Game::ScreenWidth, (float)Game::ScreenHeight),
										 m_frustrum->GetNearClipPlane(), m_frustrum->GetFarClipPlane());
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
	m_particleengine = new ParticleEngine();
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
	m_render2d = new Render2DManager();
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
		SafeDelete(m_D3D);
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
		SafeDelete(m_camera);
	}
}

void CoreEngine::ShutdownLightingManager(void)
{
	// Nothing required
}

void CoreEngine::ShutdownShaderSupport(void)
{
	// Nothing required
}

void CoreEngine::ShutdownLightShader(void)
{
	// Release the light shader object.
	if(m_lightshader)
	{
		m_lightshader->Shutdown();
		SafeDelete(m_lightshader);
	}
}

void CoreEngine::ShutdownLightFadeShader(void)
{
	// Release the light shader object.
	if(m_lightfadeshader)
	{
		m_lightfadeshader->Shutdown();
		SafeDelete(m_lightfadeshader);
	}
}

void CoreEngine::ShutdownLightHighlightShader(void)
{
	// Release the light shader object.
	if(m_lighthighlightshader)
	{
		m_lighthighlightshader->Shutdown();
		SafeDelete(m_lighthighlightshader);
	}
}

void CoreEngine::ShutdownLightHighlightFadeShader(void)
{
	// Release the light shader object.
	if (m_lighthighlightfadeshader)
	{
		m_lighthighlightfadeshader->Shutdown();
		SafeDelete(m_lighthighlightfadeshader);
	}
}

void CoreEngine::ShutdownLightFlatHighlightFadeShader(void)
{
	// Release the light shader object.
	if (m_lightflathighlightfadeshader)
	{
		m_lightflathighlightfadeshader->Shutdown();
		SafeDelete(m_lightflathighlightfadeshader);
	}
}

void CoreEngine::ShutdownParticleShader(void)
{
	if (m_particleshader)
	{
		m_particleshader->Shutdown();
		SafeDelete(m_particleshader);
	}
}

void CoreEngine::ShutdownTextureShader(void)
{
	if (m_textureshader)
	{
		m_textureshader->Shutdown();
		SafeDelete(m_textureshader);
	}
}

void CoreEngine::ShutdownFrustrum(void)
{
	// Release the view frustrum object.
	if(m_frustrum)
	{
		SafeDelete(m_frustrum);
	}
}

void CoreEngine::ShutdownAudioManager(void)
{
	// Release the audio manager object.
	if (m_audiomanager)
	{
		m_audiomanager->Shutdown();
		SafeDelete(m_audiomanager);
	}
}

void CoreEngine::ShutdownTextRendering(void)
{
	// Release the text manager object.
	if(m_textmanager)
	{
		m_textmanager->Shutdown();
		SafeDelete(m_textmanager);
	}
}

void CoreEngine::ShutdownFontShader(void)
{
	// Release the font shader object.
	if(m_fontshader)
	{
		m_fontshader->Shutdown();
		SafeDelete(m_fontshader);
	}
}

void CoreEngine::ShutdownTexcubeShader(void)
{
	if (m_texcubeshader)
	{
		m_texcubeshader->Shutdown();
		SafeDelete(m_texcubeshader);
	}
}

void CoreEngine::ShutdownFireShader(void)
{
	if (m_fireshader)
	{
		m_fireshader->Shutdown();
		SafeDelete(m_fireshader);
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
		SafeDelete(m_effectmanager);
	}
}

void CoreEngine::ShutdownSkinnedNormalMapShader(void)
{
	if (m_skinnedshader)
	{
		m_skinnedshader->Shutdown();
		SafeDelete(m_skinnedshader);
	}
}

void CoreEngine::ShutdownVolLineShader(void)
{
	// Deallocate all static data
	VolLineShader::ShutdownStaticData();

	// Deallocate the shader itself and any remaining resources
	if (m_vollineshader)
	{
		m_vollineshader->Shutdown();
		SafeDelete(m_vollineshader);
	}
}

void CoreEngine::ShutdownParticleEngine(void)
{
	if (m_particleengine)
	{
		m_particleengine->Shutdown();
		SafeDelete(m_particleengine);
	}
}

void CoreEngine::Shutdown2DRenderManager(void)
{
	if (m_render2d)
	{
		m_render2d->Shutdown();
		SafeDelete(m_render2d);
	}
}

void CoreEngine::ShutdownOverlayRenderer(void)
{
	if (m_overlayrenderer)
	{
		m_overlayrenderer->Shutdown();
		SafeDelete(m_overlayrenderer);
	}
}

void CoreEngine::ShutdownEnvironmentRendering(void)
{
	// No action required, for now
}

// Event triggered when the application window is resized
void CoreEngine::WindowResized(void)
{
	// Update any dependent calculations
	m_projscreen = XMMatrixMultiply(XMMatrixTranslation(1.0f, -1.0f, 0.0f), XMMatrixScaling(Game::ScreenWidth * 0.5f, Game::ScreenHeight * -0.5f, 1.0f));

}

// Set the visibility state of the system cursor
void CoreEngine::SetSystemCursorVisibility(bool cursor_visible)
{
	ShowCursor(cursor_visible ? TRUE : FALSE);
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
	r_viewproj = XMMatrixMultiply(r_view, r_projection);
	r_invviewproj = XMMatrixInverse(NULL, r_viewproj);
	r_viewprojscreen = XMMatrixMultiply(r_viewproj, m_projscreen);
	r_invviewprojscreen = XMMatrixInverse(NULL, r_viewprojscreen);

	// Store local float representations of each key matrix for runtime efficiency
	XMStoreFloat4x4(&r_view_f, r_view);
	XMStoreFloat4x4(&r_invview_f, r_invview);
	XMStoreFloat4x4(&r_projection_f, r_projection);
	XMStoreFloat4x4(&r_orthographic_f, r_orthographic);
	XMStoreFloat4x4(&r_viewproj_f, r_viewproj);
	XMStoreFloat4x4(&r_invviewproj_f, r_invviewproj);

	// Validate render cycle parameters before continuing with the render process
	if (!r_devicecontext) return;

	// Construct the view frustrum for this frame so we can perform culling calculations
	m_frustrum->ConstructFrustrum(r_view, r_invview);

	// Determine which system we are currently rendering
	SpaceSystem & system = Game::Universe->GetCurrentSystem();

	// Initialise the lighting manager for this frame
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Analysing frame lighting");
	RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_Render_AnalyseFrameLighting)
	{
		LightingManager.AnalyseNewFrame();
	}
	RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_Render_AnalyseFrameLighting);

	/*** Perform rendering that is common to all player environment & states ***/

	// Render the system region
	RJ_FRAME_PROFILER_CHECKPOINT("Render: System region");
	if (RenderStageActive(RenderStage::Render_SystemRegion)) 
		RenderSystemRegion();			// The system-wide rendering, e.g. space backdrop and scenery

	// Render the immmediate player region, e.g. localised space dust
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Immediate region");
	if (RenderStageActive(RenderStage::Render_ImmediateRegion)) 
		RenderImmediateRegion();

	// Render all objects in the current system; simulation state & visibility will be taken 
	// into account to ensure we only render those items that are necessary
	{
		// Render all objects
		RJ_FRAME_PROFILER_CHECKPOINT("Render: System objects");
		if (RenderStageActive(RenderStage::Render_SystemObjects))
			RenderAllSystemObjects(system);

		// Render all visible basic projectiles
		RJ_FRAME_PROFILER_CHECKPOINT("Render: Basic projectiles");
		if (RenderStageActive(RenderStage::Render_BasicProjectiles))
			RenderProjectileSet(system.Projectiles);
	}

	// Render effects and particle emitters
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Effects");
	if (RenderStageActive(RenderStage::Render_Effects)) RenderEffects();
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Particles");
	if (RenderStageActive(RenderStage::Render_ParticleEmitters)) RenderParticleEmitters();

	// Perform all 2D rendering of text and UI components
	RJ_FRAME_PROFILER_CHECKPOINT("Render: User interface");
	if (RenderStageActive(RenderStage::Render_UserInterface))
		RenderUserInterface();	

	// Perform any debug/special rendering 
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Debug data");
	if (RenderStageActive(RenderStage::Render_DebugData))
		RenderDebugData();

	// Activate the render queue optimiser here if it is ready for its next cycle
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Optimising render queue");
	if (m_rq_optimiser.Ready()) m_rq_optimiser.Run();

	// Final activity: process all items queued for rendering.  Any item that benefits from instanced/batched geometry rendering is
	// added to the render queue during the process above.  We now use instanced rendering on the entire render queue.  The more 
	// high-volume items that can be moved to use instanced rendering the better
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Processing render queue");
	ProcessRenderQueue();

	// End the frame
	RJ_FRAME_PROFILER_CHECKPOINT("Render: Ending frame");
	LightingManager.EndFrame();
}

// Processes all items in the render queue using instanced rendering, to minimise the number of render calls required per frame
RJ_PROFILED(void CoreEngine::ProcessRenderQueue, void)
{
	RM_ModelInstanceData::iterator mi, mi_end;
	D3D11_MAPPED_SUBRESOURCE mappedres;
	int instancecount, inst, n;
#	ifdef RJ_ENABLE_FRAME_PROFILER
	Timers::HRClockTime render_time;
#	endif

	// Debug output of render queue contents, if applicable
	RJ_FRAME_PROFILER_EXECUTE(DebugOutputRenderQueueContents();)

	// Iterate through each shader in the render queue
	for (int i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		// Get a reference to this specific render queue 
		RM_InstancedShaderDetails & rq_shader = m_renderqueueshaders[i];
		RJ_FRAME_PROFILER_OUTPUT(concat("Activating shader ")(i)(" [")
			((rq_shader.RequiresZSorting ? rq_shader.SortedInstances.size() : m_renderqueue[i].size()))(" models]\n").str().c_str())
			
		// Skip this shader immediately if there are no models/instances to be rendered by it (different check depending on whether this is a z-sorted shader)
		if (rq_shader.RequiresZSorting == false)	{ if (m_renderqueue[i].empty()) continue; }
		else										{ if (rq_shader.SortedInstances.empty()) continue; }

		// Set any engine properties required by this specific shader
		if (rq_shader.AlphaBlendRequired != m_D3D->GetAlphaBlendState())
			m_D3D->SetAlphaBlendState(rq_shader.AlphaBlendRequired);

		// If this is a shader that requires z-sorting, perform that sort and render by the sorted method.  We can 
		// then skip the remainder of the process for this shader and move on to the next one
		if (rq_shader.RequiresZSorting) { PerformZSortedRenderQueueProcessing(rq_shader); continue; }

		// Iterate through each model queued for rendering by this shader
		mi_end = m_renderqueue[i].end();
		for (mi = m_renderqueue[i].begin(); mi != mi_end; ++mi)
		{
			// Store a reference to the model buffer currently being rendered
			m_current_modelbuffer = mi->first;

			// Get the number of instances to be rendered
			instancecount = (int)mi->second.InstanceData.size();
			RJ_FRAME_PROFILER_OUTPUT(concat("Activating model \"")(mi->first->GetCode())("\" (")(&(mi->first))(") [")(instancecount)(" instances]\n").str().c_str())
			if (instancecount == 0) continue;

			// Loop through the instances in batches, if the total count is larger than our limit
			for (inst = 0; inst < instancecount; inst += Game::C_INSTANCED_RENDER_LIMIT)
			{
				// Record render time if profiling is enabled
				RJ_FRAME_PROFILER_EXECUTE(render_time = Timers::GetHRClockTime();)

				// Determine the number of instances to render; either the per-batch limit, or fewer if we do not have that many
				n = min(instancecount - inst, Game::C_INSTANCED_RENDER_LIMIT);

				// Update the instance buffer by mapping, updating and unmapping the memory
				memset(&mappedres, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
				r_devicecontext->Map(m_instancebuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedres);
				memcpy(mappedres.pData, &(mi->second.InstanceData[inst]), sizeof(RM_Instance) * n);
				r_devicecontext->Unmap(m_instancebuffer, 0);

				// Update the model VB pointer and then set vertex buffer data
				m_instancedbuffers[0] = m_current_modelbuffer->VertexBuffer;
				m_instancedstride[0] = m_current_modelbuffer->GetVertexSize();
				r_devicecontext->IASetVertexBuffers(0, 2, m_instancedbuffers, m_instancedstride, m_instancedoffset);

				// Set the model index buffer to active in the input assembler
				r_devicecontext->IASetIndexBuffer(m_current_modelbuffer->IndexBuffer, /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

				// Set the type of primitive that should be rendered from this vertex buffer, if it differs from the current topology
				if (rq_shader.PrimitiveTopology != m_current_topology) 
					r_devicecontext->IASetPrimitiveTopology(rq_shader.PrimitiveTopology);

				// Now process all instanced / indexed vertex data through this shader
				rq_shader.Shader->Render(	r_devicecontext, m_current_modelbuffer->GetIndexCount(),
											m_current_modelbuffer->GetIndexCount(), n,
											r_view, r_projection, m_current_modelbuffer->GetTextureResource());

				// Increment the count of draw calls that have been processed
				++m_renderinfo.DrawCalls;

				// Record profiling information if enabled this frame
				RJ_FRAME_PROFILER_OUTPUT(concat("> Rendering batch ")(inst)(" to ")(n - inst)(" [Shader=")(i)(", Model=\"")(mi->first->GetCode())
					("\"] = ")(Timers::GetMillisecondDuration(render_time, Timers::GetHRClockTime()))("ms\n").str().c_str())
			}
			
			// Finally, clear the instance vector for this shader/model now that we have fully processed it
			mi->second.InstanceData.clear();
		}
	}	

	// Return any render parameters to their default if required, to avoid any downstream impact
	if (m_D3D->GetAlphaBlendState() != D3DMain::AlphaBlendState::AlphaBlendDisabled) m_D3D->SetAlphaBlendModeDisabled();
}

// Performs an intermediate z-sorting of instances before populating and processing the render queue.  Used only for 
// shaders/techniques (e.g. alpha blending) that require instances to be z-sorted.  Takes the place of normal rendering
void CoreEngine::PerformZSortedRenderQueueProcessing(RM_InstancedShaderDetails & shader)
{
	int n;
	vector<RM_Instance> renderbuffer;
	D3D11_MAPPED_SUBRESOURCE mappedres;
#	ifdef RJ_ENABLE_FRAME_PROFILER
	Timers::HRClockTime render_time;
#	endif

	// See whether there are any instances to be rendered
	int size = (int)shader.SortedInstances.size();
	if (size == 0) return;

	// Sort the vector by z-order.  Uses default "operator<" defined in the RM_ZSortedInstance struct
	RJ_FRAME_PROFILER_EXECUTE(render_time = Timers::GetHRClockTime();)
	std::sort(shader.SortedInstances.begin(), shader.SortedInstances.end());
	RJ_FRAME_PROFILER_OUTPUT(concat("Depth-sorting ")(size)(" instances = ")(Timers::GetMillisecondDuration(render_time, Timers::GetHRClockTime()))("ms\n").str().c_str())

	// Now reverse iterate through the newly-sorted items in the vector, to pull instances in decreasing distance from the camera
	// Deliberately go to -1, so we can render the final element(s).  Loop will run from (n-1) to -1

	// The starting model will be that of the last element (which we know exists since size>0)
	m_current_modelbuffer = shader.SortedInstances[size - 1].ModelPtr;
	for (int i = size - 1; i >= -1; --i)
	{
		// If this is an instance of the same model as the previous item, and this is not the final (-1) dummy item,
		// add another element to the render buffer
		if (i != -1 && shader.SortedInstances[i].ModelPtr == m_current_modelbuffer)
		{
			renderbuffer.push_back(shader.SortedInstances[i].Item);
		}

		// If this is an instance of a different model, or is the dummy end-element, we want to render the buffer that has been accumulated so far
		if (i == -1 || shader.SortedInstances[i].ModelPtr != m_current_modelbuffer)
		{
			// We are at this point because (a) we are at the end of the vector, or (b) the model has changed for a valid reason
			// We therefore want to render the buffer now.  Make sure that the buffer actually contains items 
			n = (int)renderbuffer.size();
			if (n > 0)
			{
				// Record the render time if profiling is enabled this frame
				RJ_FRAME_PROFILER_EXECUTE(render_time = Timers::GetHRClockTime();)
				RJ_FRAME_PROFILER_EXECUTE(if (n > Game::C_INSTANCED_RENDER_LIMIT) OutputDebugString(concat("WARNING: Sorted instance count of ")(n)
					(" exceeds maximum permitted ")(Game::C_INSTANCED_RENDER_LIMIT)(" instances\n").str().c_str());)

				// Make sure we are not over the instance limit.  If we are, simply truncate.  Should not be rendering many items this way anyway
				n = min(n, Game::C_INSTANCED_RENDER_LIMIT);

				// Update the instance buffer by mapping, updating and unmapping the memory
				memset(&mappedres, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
				r_devicecontext->Map(m_instancebuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedres);
				memcpy(mappedres.pData, &(renderbuffer[0]), sizeof(RM_Instance) * n);
				r_devicecontext->Unmap(m_instancebuffer, 0);

				// Update the model VB pointer and then set vertex buffer data
				m_instancedbuffers[0] = m_current_modelbuffer->VertexBuffer;
				m_instancedstride[0] = m_current_modelbuffer->GetVertexSize();
				r_devicecontext->IASetVertexBuffers(0, 2, m_instancedbuffers, m_instancedstride, m_instancedoffset);

				// Set the model index buffer to active in the input assembler
				r_devicecontext->IASetIndexBuffer(m_current_modelbuffer->IndexBuffer, /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

				// Set the type of primitive that should be rendered from this vertex buffer, if it differs from the current topology
				if (shader.PrimitiveTopology != m_current_topology)
					r_devicecontext->IASetPrimitiveTopology(shader.PrimitiveTopology);

				// Now process all instanced / indexed vertex data through this shader
				shader.Shader->Render(	r_devicecontext, m_current_modelbuffer->GetIndexCount(), m_current_modelbuffer->GetIndexCount(), n,
										r_view, r_projection, m_current_modelbuffer->GetTextureResource());

				// Record the render time if profiling is enabled for this frame
				RJ_FRAME_PROFILER_OUTPUT(concat("> Rendering sorted batch of ")(n)(" instances [Model=")(m_current_modelbuffer->GetCode())("] = ")
					(Timers::GetMillisecondDuration(render_time, Timers::GetHRClockTime()))("ms\n").str().c_str())

				// Increment the count of draw calls that have been processed
				++m_renderinfo.DrawCalls;
			}
			
			// Clear the render buffer now that it has been rendered
			renderbuffer.clear();

			// We are either here because the model has changed, or we are at the -1 element.  If we are not at the -1 element
			// we now want to update the current model pointer, and add the current element to the render buffer as the first item
			if (i != -1)
			{
				m_current_modelbuffer = shader.SortedInstances[i].ModelPtr;
				renderbuffer.push_back(shader.SortedInstances[i].Item);
			}
		}
	}

	// Clear the sorted instance vector ready for the next frame
	shader.SortedInstances.clear();
}

// Generic iObject rendering method; used by subclasses wherever possible
void CoreEngine::RenderObject(iObject *object)
{
	if (!object) return;

	// Test whether this object is within the viewing frustrum; if not, no need to render it
	if (!m_frustrum->TestObjectVisibility(object)) return;

	// Mark the object as visible
	Game::MarkObjectAsVisible(object);

	// We are rendering this object, so call its pre-render update method
	object->PerformRenderUpdate();

	// Set the lighting configuration to be used for rendering this object
	LightingManager.SetActiveLightingConfigurationForObject(object);

    // Render either articulated or static model depending on object properties
	if (object->GetArticulatedModel())
	{
		RenderObjectWithArticulatedModel(object);
	}
	else
	{
		RenderObjectWithStaticModel(object);
	}

}

// Render an object with a static model.  Protected; called only from RenderObject()
void CoreEngine::RenderObjectWithStaticModel(iObject *object)
{
    // Guaranteed: object != NULL, based on validation in RenderModel method, which is the only method which can 
    // invoke this one. Update to include NULL checks if this situation changes

    // Make sure this object has a valid model; if not, there is nothing to render
	if (!object->GetModel()) return;

	// Add this object to the render queue for the relevant shader
	if (object->Fade.AlphaIsActive())
	{
		// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
		float alpha = object->Fade.GetFadeAlpha();
		if (alpha < Game::C_EPSILON) return;
		m_instanceparams.x = alpha;

		SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, object->GetModel(), object->GetWorldMatrix(), m_instanceparams, object->GetPosition());
	}
	else
	{
		if (object->Highlight.IsActive())
		{
			m_instanceparams = object->Highlight.GetColour();
			SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, object->GetModel(), object->GetWorldMatrix(), m_instanceparams);
		}
		else
		{
			SubmitForRendering(RenderQueueShader::RM_LightShader, object->GetModel(), object->GetWorldMatrix());
		}
	}
}

// Render an object with an articulated model.  Protected; called only from RenderObject()
void CoreEngine::RenderObjectWithArticulatedModel(iObject *object)
{
	// Guaranteed: object != NULL, object->GetArticulatedModel() != NULL,  based on validation in RenderModel 
    // method, which is the only method which can invoke this one.  Update to include NULL checks if this situation changes

    // Perform an update of the articulated model to ensure all components are correctly positioned
	ArticulatedModel *model = object->GetArticulatedModel();
	model->Update(object->GetPosition(), object->GetOrientation(), object->GetWorldMatrix());
	
    // Cache data for efficiency
    int n = model->GetComponentCount();

	// Add this object to the render queue for the relevant shader
	if (object->Fade.AlphaIsActive())
	{
		// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
		float alpha = object->Fade.GetFadeAlpha();
		if (alpha < Game::C_EPSILON) return;
		m_instanceparams.x = alpha;

        // Submit each component for rendering in turn
		ArticulatedModelComponent **component = model->GetComponents();
		for (int i = 0; i < n; ++i, ++component)
		{
			SubmitForZSortedRendering(	RenderQueueShader::RM_LightFadeShader, (*component)->Model, (*component)->GetWorldMatrix(), 
										m_instanceparams, (*component)->GetPosition());
		}
	}
	else
	{
		if (object->Highlight.IsActive())
		{
			m_instanceparams = object->Highlight.GetColour();

			// Submit each component for rendering in turn
			ArticulatedModelComponent **component = model->GetComponents();
			for (int i = 0; i < n; ++i, ++component)
			{
				SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, (*component)->Model, (*component)->GetWorldMatrix(), m_instanceparams);
			}
		}
		else
		{
			// Submit each component for rendering in turn
			ArticulatedModelComponent **component = model->GetComponents();
			for (int i = 0; i < n; ++i, ++component)
			{
				SubmitForRendering(RenderQueueShader::RM_LightShader, (*component)->Model, (*component)->GetWorldMatrix());
			}
		}
	}
}

// Renders all objects in the specified system, based on simulation state and visibility testing
void CoreEngine::RenderAllSystemObjects(SpaceSystem & system)
{
	iSpaceObject *object;

	// Iterate through all objects in the system object collection
	std::vector<ObjectReference<iSpaceObject>>::iterator it_end = system.Objects.end();
	for (std::vector<ObjectReference<iSpaceObject>>::iterator it = system.Objects.begin(); it != it_end; ++it)
	{
		// Get a reference to the object and make sure it is valid
		object = (*it)(); if (!object) continue;

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

				// Basic object types are directly pushed to the render queue using the default model rendering method
				case iObject::ProjectileObject:
					RenderObject(object);								break;
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

	// We only need to render the ship & its contents if at least one ship section was rendered
	if (shiprendered)
	{
		// Set one active lighting configuration for the entire ship (rather than by turret) for efficiency
		LightingManager.SetActiveLightingConfigurationForObject(ship);

		// Render any turret objects on the exterior of the ship, if applicable
		if (ship->TurretController.IsActive()) RenderTurrets(ship->TurretController);

		// Pass control to the environment-rendering logic to render all visible objects within the environment, if applicable.
		// Criteria are that either (a)it was requested, (b) the ship is or contains a simulation hub, or (c) the flag is set 
		// that forces the interior to always be rendered.  In addition, we must have actually rendered some part of the ship.
		if (renderinterior || is_hub || ship->InteriorShouldAlwaysBeRendered())
		{
			RenderObjectEnvironment(ship);
		}

		// Mark the ship to indicate that it was visible this frame
		Game::MarkObjectAsVisible(ship);

		// Increment the complex ship render count if any of its sections were rendered this frame
		++m_renderinfo.ComplexShipRenderCount;
	}
}
	
// Render a complex ship section to the space environment, as part of the rendering of the complex ship itself
void CoreEngine::RenderComplexShipSection(ComplexShip *ship, ComplexShipSection *sec)
{
	// Render the exterior of the ship, unless we have the relevant render flag set
	if (!m_renderflags[CoreEngine::RenderFlag::DisableHullRendering])
	{
		// Simply pass control to the main object rendering method
		RenderObject(sec);
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
	m_cache_zeropoint = XMVector3TransformCoord(NULL_VECTOR, environment->GetZeroPointWorldMatrix());

	// Also calculate a set of effective 'basis' vectors, that indicate the change in world space 
	// required to move in +x, +y and +z directions
	m_cache_el_inc[0].value = XMVector3TransformCoord(m_cache_el_inc_base[0].value, environment->GetOrientationMatrix());
	m_cache_el_inc[1].value = XMVector3TransformCoord(m_cache_el_inc_base[1].value, environment->GetOrientationMatrix());
	m_cache_el_inc[2].value = XMVector3TransformCoord(m_cache_el_inc_base[2].value, environment->GetOrientationMatrix());
	
	// Render all visible tiles
	ComplexShipTile *tile;
	iContainsComplexShipTiles::ComplexShipTileCollection::iterator it_end = environment->GetTiles().end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator it = environment->GetTiles().begin(); it != it_end; ++it)
	{
		// Get a reference to this tile
		tile = (*it).value; if (!tile) continue;

		// Make sure the tile is visible
		if (!m_frustrum->CheckSphere(XMVector3TransformCoord(NULL_VECTOR,
			XMMatrixMultiply(tile->GetWorldMatrix(), environment->GetZeroPointWorldMatrix())),
			tile->GetBoundingSphereRadius())) continue;

		// Render the tile
		RenderComplexShipTile(tile, environment);
	}

	// We perform a non-recursive vector traversal of the environment tree, and render the contents of any leaf nodes 
	// that are visible to the user
	EnvironmentTree *node;
	m_tmp_envnodes.clear();
	m_tmp_envnodes.push_back(environment->SpatialPartitioningTree);
	while (!m_tmp_envnodes.empty())
	{
		// Get the next node to be processed
		node = m_tmp_envnodes.back(); 
		m_tmp_envnodes.pop_back();
		if (!node) continue;

		// Determine the centre point of this node in world space
		//XMFLOAT3 fcentre = Game::ElementLocationToPhysicalPositionF(node->GetElementCentre());
		/*XMFLOAT3 fcentre = node->GetElementCentre().ToFloat3();
		XMVECTOR centre = XMVectorAdd(XMVectorAdd(XMVectorAdd(
			m_cache_zeropoint, XMVectorScale(m_cache_el_inc[0].value, fcentre.x)),
			XMVectorScale(m_cache_el_inc[1].value, fcentre.y)), XMVectorScale(m_cache_el_inc[2].value, fcentre.z));*/
		XMVECTOR centre = XMVector3TransformCoord(node->GetActualCentrePoint(), environment->GetZeroPointWorldMatrix());

		// We only continue with this node (and any possible children) if it is visible
		if (!m_frustrum->CheckSphere(centre, node->GetBoundingSphereRadius())) continue;

		// Test whether this is a branch or a leaf
		if (node->GetChildCount() != 0)
		{
			// This is a visible branch node, so add all its children to the search vector
			m_tmp_envnodes.insert(m_tmp_envnodes.end(), node->GetActiveChildNodes().begin(), node->GetActiveChildNodes().end());
		}
		else
		{
			// This is a leaf node; render all objects within the node
			RenderObjectEnvironmentNodeContents(environment, node);
		}
	}
}

// Renders the entire contents of an environment tree node.  Internal method; no parameter checking
void CoreEngine::RenderObjectEnvironmentNodeContents(iSpaceObjectEnvironment *environment, EnvironmentTree *node)
{
	// Render all objects within this node
	iEnvironmentObject *object;
	std::vector<iEnvironmentObject*>::const_iterator o_it_end = node->GetNodeObjects().end();
	for (std::vector<iEnvironmentObject*>::const_iterator o_it = node->GetNodeObjects().begin(); o_it != o_it_end; ++o_it)
	{
		// Get a reference to the object and make sure it is valid
		object = (*o_it); if (!object) continue;

		// Pass to different methods depending on the type of object
		switch (object->GetObjectType())
		{
			case iObject::ActorObject:
				QueueActorRendering((Actor*)object);				break;
		}
	}

	// Render all terrain within the node
	StaticTerrain *terrain;
	std::vector<StaticTerrain*>::const_iterator t_it_end = node->GetNodeTerrain().end();
	for (std::vector<StaticTerrain*>::const_iterator t_it = node->GetNodeTerrain().begin(); t_it != t_it_end; ++t_it)
	{
		// Get a reference to the object and make sure it is valid, has a model etc.
		terrain = (*t_it);
		if (!terrain || !terrain->GetDefinition() || !terrain->GetDefinition()->GetModel()) continue;

		// We should not render anything if the object has been destroyed
		if (terrain->IsDestroyed()) continue;

		// We want to render this terrain object; compose the terrain world matrix with its parent environment world matrix to get the final transform
		// Submit directly to the rendering pipeline.  Terrain objects are (currently) just a static model
		SubmitForRendering(RenderQueueShader::RM_LightShader, terrain->GetDefinition()->GetModel(),
			XMMatrixMultiply(terrain->GetWorldMatrix(), environment->GetZeroPointWorldMatrix()));
		++m_renderinfo.TerrainRenderCount;
	}
}

// Render a complex ship tile to the space environment, relative to its parent ship object
void CoreEngine::RenderComplexShipTile(ComplexShipTile *tile, iSpaceObjectEnvironment *environment)
{
	// Parameter check
	if (!tile) return;

	// Do not render anything if the tile has been destroyed (TODO: in future, render "destroyed" representation
	// of the tile and its contents instead
	if (tile->IsDestroyed()) return;

	// Calculate the absolute world matrix for this tile as (WM = Child * Parent)
	XMMATRIX world = XMMatrixMultiply(tile->GetWorldMatrix(), environment->GetZeroPointWorldMatrix());

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
				float alpha = tile->Fade.GetFadeAlpha();
				if (alpha < Game::C_EPSILON) return;
				m_instanceparams.x = alpha;

				SubmitForZSortedRendering(	RenderQueueShader::RM_LightFadeShader, tile->GetModel(), world, m_instanceparams, 
											world.r[3]);	// Position can be taken from trans. components of world matrix (_41 to _43)
			}
			else
			{
				if (tile->Highlight.IsActive())
				{
					m_instanceparams = tile->Highlight.GetColour();
					SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, tile->GetModel(), world, m_instanceparams);
				}
				else
				{
					SubmitForRendering(RenderQueueShader::RM_LightShader, tile->GetModel(), world);
				}
			}
		}
	}
	else
	{	
		XMMATRIX modelwm; 

		// Determine whether any effects are active on the tile; these will need to be propogated across to constituent parts here
		bool fade = false;
		if (tile->Fade.AlphaIsActive()) { fade = true; m_instanceparams.x = tile->Fade.GetFadeAlpha(); }

		// Iterate through all models to be rendered
		ComplexShipTile::TileCompoundModelSet::TileModelCollection::const_iterator it_end = tile->GetCompoundModelSet()->Models.end();
		for (ComplexShipTile::TileCompoundModelSet::TileModelCollection::const_iterator it = tile->GetCompoundModelSet()->Models.begin(); it != it_end; ++it)
		{
			// Get a reference to the model
			const ComplexShipTile::TileModel & item = (*it).value;
			
			// Apply a transformation of (ModelRotation * ModelTranslation * CurrentWorldMatrix)
			modelwm = XMMatrixMultiply(XMMatrixMultiply(item.rotmatrix, XMMatrixTranslationFromVector(item.offset)), world);
			
			// Submit the tile model for rendering using this adjusted world matrix
			if (fade)
			{
				SubmitForZSortedRendering(	RenderQueueShader::RM_LightFadeShader, item.model, modelwm, m_instanceparams,
											modelwm.r[3]);	// Take pos from the trans components of the world matrix (_41 to _43)
			}
			else
			{
				SubmitForRendering(RenderQueueShader::RM_LightShader, item.model, modelwm);
			}

		}
	}

	// Increment the render count
	++m_renderinfo.ComplexShipTileRenderCount;
}

// Render a simple ship to the space environment
RJ_PROFILED(void CoreEngine::RenderSimpleShip, SimpleShip *s)
{
	// Simply pass control to the main object rendering method
	RenderObject(s);

	// Render any turret objects on the exterior of the ship, if applicable
	if (s->TurretController.IsActive()) RenderTurrets(s->TurretController);

	// Increment the render count
	++m_renderinfo.ShipRenderCount;
}

// Renders a collection of turrets that have already been updated by their turret controller
void CoreEngine::RenderTurrets(TurretController & controller)
{
	// To store model data during render iterations
	SpaceTurret *turret;
	ArticulatedModel *model; int n;
	ArticulatedModelComponent **component;

	// Account for parent object state when rendering
	iObject *parent = controller.GetParent(); if (!parent) return;
	if (parent->Fade.AlphaIsActive())
	{
		// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
		float alpha = parent->Fade.GetFadeAlpha();
		if (alpha < Game::C_EPSILON) return;
		m_instanceparams.x = alpha;

		// Iterate through each turret in turn
		TurretController::TurretCollection::iterator it_end = controller.Turrets().end();
		for (TurretController::TurretCollection::iterator it = controller.Turrets().begin(); it != it_end; ++it)
		{
			// Get a reference to the model for this turret
			turret = (*it);
			model = turret->GetArticulatedModel(); if (!model) continue; 
			n = model->GetComponentCount();

			// Derive the turret world matrix since it is required for rendering (and we don't otherwise need it)
			turret->DetermineWorldMatrix();

			// Update the position of all model components before rendering
			model->Update(turret->GetPosition(), turret->GetOrientation(), turret->GetWorldMatrix());

			// Submit each component for rendering in turn
			component = model->GetComponents();
			for (int i = 0; i < n; ++i, ++component)
			{
				SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, (*component)->Model, (*component)->GetWorldMatrix(),
					m_instanceparams, (*component)->GetPosition());
			}
		}
	}
	else
	{
		if (parent->Highlight.IsActive())
		{
			m_instanceparams = parent->Highlight.GetColour();

			// Iterate through each turret in turn
			TurretController::TurretCollection::iterator it_end = controller.Turrets().end();
			for (TurretController::TurretCollection::iterator it = controller.Turrets().begin(); it != it_end; ++it)
			{
				// Get a reference to the model for this turret
				turret = (*it);
				model = turret->GetArticulatedModel(); if (!model) continue;
				n = model->GetComponentCount();

				// Derive the turret world matrix since it is required for rendering (and we don't otherwise need it)
				turret->DetermineWorldMatrix();

				// Update the position of all model components before rendering
				model->Update(turret->GetPosition(), turret->GetOrientation(), turret->GetWorldMatrix());

				// Submit each component for rendering in turn
				component = model->GetComponents();
				for (int i = 0; i < n; ++i, ++component)
				{
					SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, (*component)->Model, (*component)->GetWorldMatrix(), m_instanceparams);
				}
			}
		}
		else
		{
			// Iterate through each turret in turn
			TurretController::TurretCollection::iterator it_end = controller.Turrets().end();
			for (TurretController::TurretCollection::iterator it = controller.Turrets().begin(); it != it_end; ++it)
			{
				// Get a reference to the model for this turret
				turret = (*it);
				model = turret->GetArticulatedModel(); if (!model) continue;
				n = model->GetComponentCount();

				// Derive the turret world matrix since it is required for rendering (and we don't otherwise need it)
				turret->DetermineWorldMatrix();

				// Update the position of all model components before rendering
				model->Update(turret->GetPosition(), turret->GetOrientation(), turret->GetWorldMatrix());

				// Submit each component for rendering in turn
				component = model->GetComponents();
				for (int i = 0; i < n; ++i, ++component)
				{
					SubmitForRendering(RenderQueueShader::RM_LightShader, (*component)->Model, (*component)->GetWorldMatrix());
				}
			}
		}
	}
}


// Renders all elements of a projectile set which are currently visible
void CoreEngine::RenderProjectileSet(BasicProjectileSet & projectiles)
{
	// Make sure the collection is active (which guarantees that it contains >0 projectiles)
	if (!projectiles.Active) return;

	// Iterate through each element of the projectile set
	RM_Instance instance;
	for (std::vector<BasicProjectile>::size_type i = 0; i <= projectiles.LiveIndex; ++i)
	{
		// Test visibility; we will only render projectiles within the viewing frustum
		BasicProjectile & proj = projectiles.Items[i];
		if (m_frustrum->CheckSphere(proj.Position, proj.Speed) == false) continue;
		
		// Update the render instance using data from this projectile
		proj.GenerateRenderInstance(instance);

		// Submit directly for rendering using this instance data and the cached model/texture data in the projectile definition
		SubmitForZSortedRendering(RenderQueueShader::RM_VolLineShader, proj.Definition->Buffer, instance, proj.Position);
	}
}

// Rendering method for skinned models
void CoreEngine::RenderSkinnedModelInstance(SkinnedModelInstance &model)
{

}

// Renders a volumetric line
void CoreEngine::RenderVolumetricLine(const VolumetricLine & line)
{
	// Embed line endpoints within the first two rows of the instance transform
	RM_Instance m; XMFLOAT4 p1, p2;
	XMStoreFloat4(&p1, line.P1); 
	XMStoreFloat4(&p2, line.P2);

	// Store all data within the instance matrix; start pos in row 1, end pos in row 2, line colour and alpha in row 3, row4 = unused
	m.World = XMFLOAT4X4(p1.x, p1.y, p1.z, 1.0f, p2.x, p2.y, p2.z, 1.0f, line.Colour.x, line.Colour.y, line.Colour.z, line.Colour.w, 0.0f, 0.0f, 0.0f, 0.0f);

	// Provide additional parameters as required.  x = line radius
	m.Params = line.Params;
	
	// Submit to the render queue
	SubmitForZSortedRendering(RenderQueueShader::RM_VolLineShader, VolLineShader::LineModel(line.RenderTexture), m, line.P1);
}

RJ_PROFILED(void CoreEngine::RenderImmediateRegion, void)
{
	// Prepare all region particles, calculate vertex data and promote all to the buffer ready for shader rendering
	D::Regions::Immediate->Render(m_D3D->GetDeviceContext(), r_view);

	// Enable alpha blending before rendering any particles
	m_D3D->SetAlphaBlendModeEnabled();
	m_D3D->DisableZBufferWriting();

	// Render dust particles using the particle shader.  World matrix is set to the ID since all computations are in local/view space
	m_particleshader->Render( r_devicecontext, D::Regions::Immediate->GetActiveParticleCount() * 6, 
							  ID_MATRIX, r_view, r_projection, D::Regions::Immediate->GetParticleTextureResource() );

	// Disable alpha blending after rendering the particles
	// TODO: Move to somewhere more global once we have further rendering that requires alpha effects.  Avoid multiple on/off switches
	m_D3D->SetAlphaBlendModeDisabled();	
	m_D3D->EnableZBuffer();
}

RJ_PROFILED(void CoreEngine::RenderSystemRegion, void)
{
	// Use ID for the world matrix, except use the last (translation) row of the inverse view matrix for r[3] (_41 to _43)
	XMMATRIX world = ID_MATRIX;
	world.r[3] = r_invview.r[3];

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
	/*if (false) {
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
	}*/

	// Disable alpha blending after all effects are rendered
	m_D3D->SetAlphaBlendModeDisabled();
}

RJ_PROFILED(void CoreEngine::RenderParticleEmitters, void)
{
	// Pass control to the particle engine to perform all required rendering
	m_particleengine->Render(r_view, r_projection, m_D3D, m_camera);
}

// Push an actor onto the queue for rendering
void CoreEngine::QueueActorRendering(Actor *actor)
{
	if (actor)
	{
		Game::MarkObjectAsVisible(actor);
		m_queuedactors.push_back(actor);
	}
}


// Renders all actors that have been queued up for rendering
RJ_PROFILED(void CoreEngine::ProcessQueuedActorRendering, void)
{
	// Quit immediately if there are no queued actors to render
	if (m_queuedactors.empty()) return;

	// Disable rasteriser culling to prevent loss of intermediate faces in skinned models; we only have to do this once before rendering all actors
	m_D3D->DisableRasteriserCulling();

	// Get parameters that are used for every actor being rendered.  This is done once before rendering all actors, to improve efficiency
	XMFLOAT3 campos; XMStoreFloat3(&campos, m_camera->GetPosition());
	
	// Iterate through the vector of queued actors
	Actor *actor;
	std::vector<Actor*>::iterator it_end = m_queuedactors.end();
	for (std::vector<Actor*>::iterator it = m_queuedactors.begin(); it != it_end; ++it)
	{
		// Calculate new transforms for the bones & mesh of this actor
		actor = (*it);
		actor->UpdateForRendering(Game::TimeFactor);

		// Determine a lighting configuration for this actor
		LightingManager.SetActiveLightingConfigurationForObject(actor);

		// Render using the skinned model shader
		m_skinnedshader->Render(r_devicecontext, (*it)->GetModel(), campos, r_view_f, r_projection_f);
	}

	// Enable rasteriser culling again once all actors have been rendered
	m_D3D->EnableRasteriserCulling();

	// Increase the count of actors we processed; we can count every item that was in the queue, since they 
	// were validated before being placed in the queue so should all render
	m_renderinfo.ActorRenderCount += (int)m_queuedactors.size();

	// Reset the queue now that it has been processed, so we are ready for adding new items in the next frame
	m_queuedactors.clear();
}

void CoreEngine::ClearRenderingQueue(void)
{
	RM_ModelInstanceData::iterator mi, mi_end;

	// Iterate through each shader in the render queue
	for (int i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		// Iterate through each model queued for rendering by this shader
		mi_end = m_renderqueue[i].end();
		for (mi = m_renderqueue[i].begin(); mi != mi_end; ++mi)
		{
			// Clear all instance data from the associated vector
			mi->second.InstanceData.clear();
		}
	}
}

// Performs rendering of debug/special data
void CoreEngine::RenderDebugData(void)
{
	if (m_renderflags[CoreEngine::RenderFlag::RenderTree]) DebugRenderSpatialPartitioningTree();
	if (m_renderflags[CoreEngine::RenderFlag::RenderEnvTree]) DebugRenderEnvironmentTree();
	if (m_renderflags[CoreEngine::RenderFlag::RenderOBBs]) DebugRenderSpaceCollisionBoxes();
	if (m_renderflags[CoreEngine::RenderFlag::RenderTerrainBoxes]) DebugRenderEnvironmentCollisionBoxes();
	if (m_renderflags[CoreEngine::RenderFlag::RenderNavNetwork]) DebugRenderEnvironmentNavNetwork();
	if (m_renderflags[CoreEngine::RenderFlag::RenderObjectIdentifiers]) DebugRenderObjectIdentifiers();
}

// Performs debug rendering of the active spatial partitioning tree
void CoreEngine::DebugRenderSpatialPartitioningTree(void)
{
	if (Game::Universe->GetCurrentSystem().SpatialPartitioningTree != NULL)
		m_overlayrenderer->DebugRenderSpatialPartitioningTree<iObject*>(Game::Universe->GetCurrentSystem().SpatialPartitioningTree, true);
}

// Performs debug rendering of a specified environment spatial partitioning tree
void CoreEngine::DebugRenderEnvironmentTree(void)
{
	iObject *obj = Game::GetObjectByID(m_debug_renderenvtree);
	if (m_debug_renderenvtree == 0 || !obj || !obj->IsEnvironment()) { SetRenderFlag(CoreEngine::RenderFlag::RenderEnvTree, false); return; }
	iSpaceObjectEnvironment *env = (iSpaceObjectEnvironment*)obj;
	if (!env->SpatialPartitioningTree) { SetRenderFlag(CoreEngine::RenderFlag::RenderEnvTree, false); return; }

	m_overlayrenderer->DebugRenderEnvironmentTree(env->SpatialPartitioningTree, true);
}

// Performs debug rendering of all space object OBBs
void CoreEngine::DebugRenderSpaceCollisionBoxes(void)
{
	iSpaceObject *object;
	std::vector<iObject*> objects; int count = 0;
	float radius; bool invalidated;

	// Find all active space objects around the player; take a different approach depending on whether the player is in a ship or on foot
	if (Game::CurrentPlayer->GetState() == Player::StateType::OnFoot)
	{
		// Player is on foot, so use a proximity test to the object currently considered their parent environment
		if (Game::CurrentPlayer->GetParentEnvironment() == NULL) return;
		count = 1 + Game::ObjectSearch<iObject>::GetAllObjectsWithinDistance(Game::CurrentPlayer->GetParentEnvironment(), 10000.0f, objects,
																			(Game::ObjectSearchOptions::OnlyCollidingObjects));

		// Also include the parent ship environmment (hence why we +1 to the count above)
		objects.push_back((iObject*)Game::CurrentPlayer->GetParentEnvironment());
	}
	else
	{
		// Player is in a spaceobject ship, so use the proximity test on their ship
		if (Game::CurrentPlayer->GetPlayerShip() == NULL) return;
		count = 1 + Game::ObjectSearch<iObject>::GetAllObjectsWithinDistance(Game::CurrentPlayer->GetPlayerShip(), 10000.0f, objects, 
																			(Game::ObjectSearchOptions::OnlyCollidingObjects));

		// Also include the player ship (hence why we +1 to the count above)
		objects.push_back((iObject*)Game::CurrentPlayer->GetPlayerShip());
	}

	// Iterate through the active objects
	bool leaf_nodes_only = GetRenderFlag(CoreEngine::RenderFlag::RenderOBBLeafNodesOnly);
	std::vector<iObject*>::iterator it_end = objects.end();
	for (std::vector<iObject*>::iterator it = objects.begin(); it != it_end; ++it)
	{
		object = (iSpaceObject*)(*it);
		radius = object->GetCollisionSphereRadius();

		// Render the oriented bounding box(es) used for narrowphase collision detection, if applicable for this object
		// Also force an immediate recalculation of any invalidated OBBs so that they can be debug-rendered
		if (object->GetCollisionMode() == Game::CollisionMode::FullCollision)
		{
			// Test whether the OBB is invalidated and therefore needs to be recalculated
			invalidated = object->CollisionOBB.IsInvalidated();
			if (invalidated) object->CollisionOBB.UpdateFromObject(*object);

			// Render the OBB
			Game::Engine->GetOverlayRenderer()->RenderOBB(object->CollisionOBB, true, leaf_nodes_only, 
				(invalidated ? OverlayRenderer::RenderColour::RC_LightBlue : OverlayRenderer::RenderColour::RC_Green), 0.1f);
		}
	}
}

// Performs debug rendering of all environment terrain collision boxes
void CoreEngine::DebugRenderEnvironmentCollisionBoxes(void)
{
	// Parameter check
	if (m_debug_renderenvboxes == 0 || Game::ObjectExists(m_debug_renderenvboxes) == false) return;
	AXMVECTOR_P v[8]; iEnvironmentObject *a_obj; StaticTerrain *t_obj;

	// Get a reference to the environment object
	iSpaceObjectEnvironment *parent = (iSpaceObjectEnvironment*)Game::GetObjectByID(m_debug_renderenvboxes);
	if (!parent) return;

	// Iterate through all active objects within this parent environment
	std::vector<ObjectReference<iEnvironmentObject>>::iterator a_it_end = parent->Objects.end();
	for (std::vector<ObjectReference<iEnvironmentObject>>::iterator a_it = parent->Objects.begin(); a_it != a_it_end; ++a_it)
	{
		a_obj = (*a_it)(); if (!a_obj) continue;

		// Determine the location of all vertices for each bounding volume and render using the overlay renderer
		a_obj->CollisionOBB.DetermineVertices(v);
		Game::Engine->GetOverlayRenderer()->RenderCuboid(v, OverlayRenderer::RenderColour::RC_LightBlue, 0.1f);
	}

	// Iterate through all terrain objects within this parent environment
	std::vector<StaticTerrain*>::iterator t_it_end = parent->TerrainObjects.end();
	for (std::vector<StaticTerrain*>::iterator t_it = parent->TerrainObjects.begin(); t_it != t_it_end; ++t_it)
	{
		t_obj = (*t_it); if (!t_obj || t_obj->IsDestroyed()) continue;

		if (m_debug_terrain_render_mode == DebugTerrainRenderMode::Solid)
		{
			// Perform solid rendering
			Game::Engine->GetOverlayRenderer()->RenderCuboid(XMMatrixMultiply(t_obj->GetWorldMatrix(), parent->GetZeroPointWorldMatrix()),
				OverlayRenderer::RenderColour::RC_Red, XMVectorScale(XMLoadFloat3(&(t_obj->GetExtentF())), 2.0f));
		}
		else
		{
			// Determine the location of all vertices for each bounding volume and render wireframe using the overlay renderer
			t_obj->DetermineCollisionBoxVertices(parent, v);
			Game::Engine->GetOverlayRenderer()->RenderCuboid(v, OverlayRenderer::RenderColour::RC_Red, 0.1f);
		}
	}
}

// Performs debug rendering of a specified environment navigation network
void CoreEngine::DebugRenderEnvironmentNavNetwork(void)
{
	// Parameter check
	if (m_debug_renderenvnetwork == 0) return;

	// Get a reference to the environment object and make sure it exists
	iSpaceObjectEnvironment *env = (iSpaceObjectEnvironment*)Game::GetObjectByID(m_debug_renderenvnetwork);
	if (!env) return;

	// Make sure the environment has an active nav network
	NavNetwork *network = env->GetNavNetwork();
	if (!network) return;

	// All environment-relative rendering will be transformed by the environment world matrix
	XMMATRIX envworld = env->GetZeroPointWorldMatrix();
	XMVECTOR pos, tgtpos; const NavNode *tgt;

	// Get a reference to the node data and iterate through it
	int nodecount = network->GetNodeCount();
	const NavNode *nodes = network->GetNodes();
	for (int i = 0; i < nodecount; ++i)
	{
		// Get a reference to the node and transform its position into world space
		const NavNode & node = nodes[i];
		pos = XMVector3TransformCoord(VectorFromIntVector3SwizzleYZ(node.Position), envworld);
		
		// Render a marker at this location
		m_overlayrenderer->RenderNode(pos, OverlayRenderer::RenderColour::RC_Red);

		// Now we want to render connections to all connected nav nodes
		int linkcount = node.NumConnections;
		for (int c = 0; c < linkcount; ++c)
		{
			// Attempt to get the target node.  Render a connection IF our index < target index
			// This avoids rendering each connection twice.  NOTE: Assumes BI-DIRECTIONAL connections
			tgt = node.Connections[c].Target;
			if (tgt && tgt->Index > node.Index)
			{
				// Render a connection to this node
				tgtpos = XMVector3TransformCoord(VectorFromIntVector3SwizzleYZ(tgt->Position), envworld);
				m_overlayrenderer->RenderLine(pos, tgtpos, OverlayRenderer::RenderColour::RC_LightBlue, 0.25f, -1.0f);
			}
		}
	}
}

// Renders the identifier of all visible objects in the player's immediate vicinity
void CoreEngine::DebugRenderObjectIdentifiers(void)
{
	static const XMFLOAT2 DEBUG_ID_TEXT_OFFSET = XMFLOAT2(0.0f, 0.0f);
	static const XMFLOAT4 DEBUG_MAJOR_ID_TEXT_COLOUR = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);
	static const XMFLOAT4 DEBUG_MINOR_ID_TEXT_COLOUR = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	static const float DEBUG_MAJOR_ID_TEXT_SIZE = 1.0f;
	static const float DEBUG_MINOR_ID_TEXT_SIZE = 1.0f; // 0.75f;
	static const unsigned int DEBUG_ID_MAX_DISPLAYED = 128U;
	static const XMVECTOR DEBUG_TERRAIN_ID_RENDER_DIST_SQ = XMVectorReplicate(Game::ElementLocationToPhysicalPosition(4));	// Render terrain IDs within two elements of the camera

	// Maintain a static set of sentence objects that we can use for debug rendering 
	// (TODO: temporary and inefficient.  Replace when text rendering is upgraded.  We also do not dispose of these at shutdown right now)
	if (m_debug_renderobjid_text.size() == 0)
	{
		for (unsigned int i = 0; i < DEBUG_ID_MAX_DISPLAYED; ++i)
		{
			SentenceType *s = m_textmanager->CreateSentence(Game::Fonts::FONT_BASIC1, 256);
			if (!s) break;

			s->render = false;
			m_debug_renderobjid_text.push_back(s);
		}
	}

	// We can potentially select a specific object (and its contents) for rendering
	Game::ID_TYPE target = GetDebugObjectIdentifierRenderTargetObject();

	// Get the set of all objects to be rendered
	std::vector<DebugIDRenderDetails> objects;
	if (target == 0)
	{
		// Locate the spatial partitioning tree node that contains the current camera position (take
		// this longer approach to account for cases where the player is using the debug camera)
		const XMVECTOR & pos = GetCamera()->GetPosition();
		if (Game::Universe->GetCurrentSystem().SpatialPartitioningTree == NULL) return;
		Octree<iObject*> *node = Game::Universe->GetCurrentSystem().SpatialPartitioningTree->GetNodeContainingPoint(pos);
		if (!node) return;

		// Get the set of all objects nearby
		std::vector<iObject*> spaceobjects;
		int n = Game::ObjectSearch<iObject>::GetAllObjectsWithinDistance(pos, node, m_debug_renderobjid_distance, spaceobjects, Game::ObjectSearchOptions::NoSearchOptions);

		// Store required data in the object array
		for (int i = 0; i < n; ++i)
			if (spaceobjects[i] && spaceobjects[i]->IsCurrentlyVisible())
				objects.push_back(DebugIDRenderDetails(spaceobjects[i]->CollisionOBB.Data(), NULL_VECTOR, 
					spaceobjects[i]->DebugString(), DEBUG_MAJOR_ID_TEXT_COLOUR, DEBUG_MAJOR_ID_TEXT_SIZE));
	}
	else
	{
		
		// If we set a specific target object, and that object is an environment, also render idenitifers of objects within th environment
		iObject *obj = Game::GetObjectByID(target);
		if (obj) 
		{
			objects.push_back(DebugIDRenderDetails(obj->CollisionOBB.Data(), NULL_VECTOR, obj->DebugString(), 
				DEBUG_MAJOR_ID_TEXT_COLOUR, DEBUG_MAJOR_ID_TEXT_SIZE));

			if (obj->IsEnvironment())
			{
				// Render all environment objects
				iSpaceObjectEnvironment *env = (iSpaceObjectEnvironment*)obj;
				env->CollisionOBB.UpdateIfRequired();

				std::vector<ObjectReference<iEnvironmentObject>>::iterator it_o_end = env->Objects.end();
				for (std::vector<ObjectReference<iEnvironmentObject>>::iterator it_o = env->Objects.begin(); it_o != it_o_end; ++it_o) {
					if ((*it_o)() && (*it_o)()->IsCurrentlyVisible()) 
						objects.push_back(DebugIDRenderDetails((*it_o)()->CollisionOBB.Data(), NULL_VECTOR, 
						concat((*it_o)()->GetCode())(" (")((*it_o)()->GetID())(")").str(), DEBUG_MAJOR_ID_TEXT_COLOUR, DEBUG_MAJOR_ID_TEXT_SIZE));
				}
				
				// We will use a temporary OBB and precalcualted position multiplier for element contents stored in local space
				XMVECTOR pos_local = XMVector3TransformCoord(ONE_VECTOR, env->GetOrientationMatrix());
				XMVECTOR zero_pos = XMVectorSubtract(env->GetPosition(), XMVectorMultiply(pos_local, env->CollisionOBB.ConstData().ExtentV));
				const XMVECTOR & cam_pos = m_camera->GetPosition();
				OrientedBoundingBox::CoreOBBData obb;
				obb.Axis[0] = env->CollisionOBB.ConstData().Axis[0];
				obb.Axis[1] = env->CollisionOBB.ConstData().Axis[1];
				obb.Axis[2] = env->CollisionOBB.ConstData().Axis[2];
				
				// Render all tiles
				iContainsComplexShipTiles::ComplexShipTileCollection::iterator it_t_end = env->GetTiles().end();
				for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator it_t = env->GetTiles().begin(); it_t != it_t_end; ++it_t) {
					if (!(*it_t).value) continue;
					obb.Centre = XMVectorMultiplyAdd(pos_local, XMVectorAdd((*it_t).value->GetElementPosition(), XMVectorMultiply((*it_t).value->GetWorldSize(), HALF_VECTOR)), zero_pos);
					obb.UpdateExtentFromSize((*it_t).value->GetWorldSize());
					objects.push_back(DebugIDRenderDetails(obb, NULL_VECTOR, 
						concat((*it_t).value->GetCode())(" (")((*it_t).value->GetID())(")").str(), DEBUG_MAJOR_ID_TEXT_COLOUR, DEBUG_MAJOR_ID_TEXT_SIZE));
				}

				// Render all terrain objects
				iSpaceObjectEnvironment::TerrainCollection::iterator it_e_end = env->TerrainObjects.end();
				for (iSpaceObjectEnvironment::TerrainCollection::iterator it_e = env->TerrainObjects.begin(); it_e != it_e_end; ++it_e) {
					if (!(*it_e)) continue;
					obb.Centre = XMVectorMultiplyAdd(pos_local, (*it_e)->GetPosition(), zero_pos);
					if (XMVector3Greater(XMVector3LengthSq(XMVectorSubtract(cam_pos, obb.Centre)), DEBUG_TERRAIN_ID_RENDER_DIST_SQ)) continue;
					obb.UpdateExtent((*it_e)->GetExtentF());
					objects.push_back(DebugIDRenderDetails(obb, NULL_VECTOR, concat((*it_e)->GetID()).str(), 
						DEBUG_MINOR_ID_TEXT_COLOUR, DEBUG_MINOR_ID_TEXT_SIZE));
				}
			}
		}
	}

	// Iterate over each object in turn
	size_t sentence_id = 0U; size_t sentence_count = m_debug_renderobjid_text.size();
	std::vector<DebugIDRenderDetails>::const_iterator it_end = objects.end();
	for (std::vector<DebugIDRenderDetails>::const_iterator it = objects.begin(); it != it_end; ++it)
	{
		// Get a sentence object to render this identifier.  If we have used all available objects, quit here
		if (sentence_id >= sentence_count) break;
		SentenceType *text = m_debug_renderobjid_text.at(sentence_id);
		if (!text) break;
		++sentence_id;

		// Create a text object for this identifer (TODO: inefficient)
		std::string textstring = (*it).text;
		m_textmanager->UpdateSentence(text, textstring.c_str(),
			0, 0, true, (*it).text_col, (*it).text_size);

		// Calculate the screen-space position for the centre of this text, then offset so that the text is centred over it
		// Force-update the object OBB to ensure that the latest values are used
		XMVECTOR pos = XMVectorSubtract(
			GetScreenLocationForObject((*it).obb, DEBUG_ID_TEXT_OFFSET),
			XMVectorSet(text->sentencewidth * 0.5f, text->sentenceheight * 0.5f, 0.0f, 0.0f));

		// Update the sentence position by performing a full update on the sentence again.  This will
		// regenerate the text VBs.  (TODO: EVEN MORE INEFFICIENT.  Replace)
		m_textmanager->UpdateSentence(text, textstring.c_str(), (int)XMVectorGetX(pos), (int)XMVectorGetY(pos), true,
			(*it).text_col, (*it).text_size);
	}

	// Set any unused text objects to be hidden this frame
	for (size_t i = sentence_id; i < sentence_count; ++i)
	{
		if (m_debug_renderobjid_text.at(i)) m_textmanager->DisableSentenceRendering(m_debug_renderobjid_text.at(i));
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

// Calculates the bounds of this object in screen space
void CoreEngine::DetermineObjectScreenBounds(const OrientedBoundingBox::CoreOBBData & obb, XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds)
{
	// Get the bounds of this object in world space
	XMVECTOR wmin, wmax;
	obb.DetermineWorldSpaceBounds(wmin, wmax);

	// Transform these world bounds into screen space and return
	outMinBounds = WorldToScreen(wmin);
	outMaxBounds = WorldToScreen(wmax);
}

// Calculates the bounds of this object in screen space, after applying a world-space offset to the object position
void CoreEngine::DetermineObjectScreenBoundsWithWorldSpaceOffset(const OrientedBoundingBox::CoreOBBData & obb, const FXMVECTOR world_offset, 
																 XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds)
{
	// Get the bounds of this object in world space
	XMVECTOR wmin, wmax;
	obb.DetermineWorldSpaceBounds(wmin, wmax);

	// Incorporate the offset and transform these world bounds into screen space
	outMinBounds = WorldToScreen(XMVectorAdd(wmin, world_offset));
	outMaxBounds = WorldToScreen(XMVectorAdd(wmax, world_offset));
}

// Returns a position in screen space corresponding to the specified object.  Accepts an offset parameter
// in screen coordinates based on the object size; [0, +0.5] would be centred in x, and return a position
// at the top edge of the object in screen space
XMVECTOR CoreEngine::GetScreenLocationForObject(const OrientedBoundingBox::CoreOBBData & obb, const XMFLOAT2 & offset)
{
	// Get the object bounds in screen space
	XMVECTOR smin, smax;
	DetermineObjectScreenBounds(obb, smin, smax);

	// Offset will be a linear interpolation between min and max.  We define [0,0] as the centre point, so
	// increase each offset value by 0.5f such that [0,0] moves from bottom-left to centre point
	return XMVectorLerpV(smin, smax, XMVectorSet(offset.x + 0.5f, offset.y + 0.5f, 0.5f, 0.5f));
}

// Returns a position in screen space corresponding to the specified object, after applying the specified
// world offset to object position.  Accepts an offset parameter in screen coordinates based on the 
// object size; [0, +0.5] would be centred in x, and return a position at the top edge of the object in screen space
XMVECTOR CoreEngine::GetScreenLocationForObjectWithWorldOffset(const OrientedBoundingBox::CoreOBBData & obb, 
															   const FXMVECTOR world_offset, const XMFLOAT2 & offset)
{
	// Get the object bounds in screen space
	XMVECTOR smin, smax;
	DetermineObjectScreenBoundsWithWorldSpaceOffset(obb, world_offset, smin, smax);

	// Offset will be a linear interpolation between min and max.  We define [0,0] as the centre point, so
	// increase each offset value by 0.5f such that [0,0] moves from bottom-left to centre point
	return XMVectorLerpV(smin, smax, XMVectorSet(offset.x + 0.5f, offset.y + 0.5f, 0.5f, 0.5f));
}

// Initialise static collection of basic colour data
const std::array<BasicColourDefinition, 8> CoreEngine::BASIC_COLOURS =
{
	BasicColourDefinition(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), "Red"),
	BasicColourDefinition(XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow"),
	BasicColourDefinition(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), "Green"),
	BasicColourDefinition(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), "Blue"),
	BasicColourDefinition(XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), "Orange"),
	BasicColourDefinition(XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), "Cyan"),
	BasicColourDefinition(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), "Purple"),
	BasicColourDefinition(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), "White")
};


// Virtual inherited method to accept a command from the console
bool CoreEngine::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "render_tree")
	{
		if (Game::Universe->GetCurrentSystem().SpatialPartitioningTree == NULL)
			{ command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::NoSpatialPartitioningTreeToRender, 
				"No tree to render"); return true; }

		bool b = (command.ParameterAsBool(0));
		SetRenderFlag(CoreEngine::RenderFlag::RenderTree, b);
		command.SetSuccessOutput(concat((b ? "Enabling" : "Disabling"))(" render of spatial partitioning tree").str());
		return true;
	}
	else if (command.InputCommand == "render_envtree")
	{
		size_t pcount = command.InputParameters.size();
		bool render = false; iSpaceObjectEnvironment *env = NULL;
		if (pcount == 1U)
		{
			render = (command.ParameterAsBool(0));
			if (!Game::CurrentPlayer || !Game::CurrentPlayer->GetActor() || !Game::CurrentPlayer->GetActor()->GetParentEnvironment())
			{ command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::NoValidEnvironmentSpecified,
					"Cannot determine valid environment tree to render"); return true; }
			env = Game::CurrentPlayer->GetActor()->GetParentEnvironment();
		}
		else if (pcount == 2U)
		{
			render = (command.Parameter(1) == "1");

			iObject *obj = Game::GetObjectByInstanceCode(command.Parameter(0));
			if (!obj || !obj->IsEnvironment())
			{
				command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
					concat("Invalid object \"")(command.Parameter(0))("\" specified").str()); return true;
			}
			env = (iSpaceObjectEnvironment*)obj;
		}

		if (!render)
		{
			SetRenderFlag(CoreEngine::RenderFlag::RenderEnvTree, false);
			SetDebugTreeRenderEnvironment(0);
			command.SetSuccessOutput("Disabling render of environment spatial partitioning tree");
			return true;
		}
		else
		{
			if (!env) { command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
					concat("Invalid or missing environment specified").str()); return true; }
			SetRenderFlag(CoreEngine::RenderFlag::RenderEnvTree, true);
			SetDebugTreeRenderEnvironment(env->GetID());
			command.SetSuccessOutput("Enabling render of environment spatial partitioning tree");
			return true;
		}
	}
	else if (command.InputCommand == "hull_render")
	{
		bool b = !(command.ParameterAsBool(0));
		SetRenderFlag(CoreEngine::RenderFlag::DisableHullRendering, b);
		command.SetSuccessOutput(concat((b ? "Disabling" : "Enabling"))(" rendering of ship hulls").str()); return true;
	}
	else if (command.InputCommand == "render_obb")
	{
		bool render = (command.ParameterAsBool(0));
		bool leaf_only = (command.ParameterAsBool(1) & render);		// We can only set this flag if "render" is also true
		SetRenderFlag(CoreEngine::RenderFlag::RenderOBBs, render);
		SetRenderFlag(CoreEngine::RenderFlag::RenderOBBLeafNodesOnly, leaf_only);

		command.SetSuccessOutput(concat((render ? "Enabling" : "Disabling"))(" rendering of object OBBs")
			(leaf_only ? " (leaf nodes only)" : "").str()); return true;
	}
	else if (command.InputCommand == "render_terrainboxes")
	{
		if (command.ParameterAsBool(1) == true)
		{
			iObject *env = Game::GetObjectByInstanceCode(command.Parameter(0));
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
	else if (command.InputCommand == "fade_interior")
	{
		iObject *obj = Game::GetObjectByInstanceCode(command.Parameter(0));
		if (!obj || !obj->IsEnvironment())
		{
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
				concat("Invalid object \"")(command.Parameter(0))("\" specified").str()); return true;
		}
		iSpaceObjectEnvironment *env = (iSpaceObjectEnvironment*)obj;

		if (command.ParameterAsBool(1) == true)
		{
			int n = env->GetTileCount();
			for (int i = 0; i < n; ++i)
			{
				ComplexShipTile *t = env->GetTile(i); if (!t) continue;
				t->Fade.FadeToAlpha(1.0f + ((float)i / (float)n), 0.1f);
			}
			command.SetSuccessOutput(concat("Fading out environment interior for ")(env->GetInstanceCode()).str()); return true;
		}
		else
		{
			int n = env->GetTileCount();
			for (int i = 0; i < n; ++i)
			{
				ComplexShipTile *t = env->GetTile(i); if (!t) continue;
				t->Fade.FadeIn(1.0f + ((float)i / (float)n));
			}
			command.SetSuccessOutput(concat("Fading environment interior of ")(env->GetInstanceCode())(" back in").str()); return true;
		}
	}
	else if (command.InputCommand == "terrain_debug_render_mode")
	{
		if (command.InputParameters.size() < 1) {
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::RequiredCommandParametersNotProvided,
				"Terrain render mode not specified"); 
		}
		if (command.Parameter(0) == "solid") {
			m_debug_terrain_render_mode = DebugTerrainRenderMode::Solid;
			command.SetSuccessOutput("Setting debug terrain render mode to solid-render");
		}
		else {
			m_debug_terrain_render_mode = DebugTerrainRenderMode::Normal;
			command.SetSuccessOutput("Setting debug terrain render mode to wireframe");
		}
		return true;
	}
	else if (command.InputCommand == "render_navnetwork")
	{
		if (command.ParameterAsBool(1) == true)
		{
			iObject *env = Game::GetObjectByInstanceCode(command.Parameter(0));
			if (!env || !env->IsEnvironment())
			{
				command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
					concat("Invalid object \"")(command.Parameter(0))("\" specified").str()); return true;
			}

			SetRenderFlag(CoreEngine::RenderFlag::RenderNavNetwork, true);
			SetDebugNavNetworkRenderEnvironment(env->GetID());
			command.SetSuccessOutput("Enabling render of environment nav network");
		}
		else
		{
			SetRenderFlag(CoreEngine::RenderFlag::RenderNavNetwork, false);
			m_debug_renderenvnetwork = 0;
			command.SetSuccessOutput("Disabling render of environment nav network");
		}
		return true;
	}
	else if (command.InputCommand == "render_identifiers")
	{
		bool b = command.ParameterAsBool(0);
		SetRenderFlag(CoreEngine::RenderFlag::RenderObjectIdentifiers, b);

		// If we are disabling the render of IDs, hide any existing text objects that are being used
		if (!b)
		{
			std::vector<SentenceType*>::iterator it_end = m_debug_renderobjid_text.end();
			for (std::vector<SentenceType*>::iterator it = m_debug_renderobjid_text.begin(); it != it_end; ++it)
				if (*it) (*it)->render = false;
		}

		// Render distance is an optional parameter
		float dist = command.ParameterAsFloat(1);
		if (dist == 0.0f) dist = 10000.0f;
		SetDebugObjectIdentifierRenderingDistance(dist);
		
		// Specific target object (usually an environment) is an optional parameter
		iObject *target = command.ParameterAsObject(2);
		SetDebugObjectIdentifierRenderTargetObject(target ? target->GetID() : 0);

		command.SetSuccessOutput(concat((b ? "Enabling" : "Disabling"))(" render of object identifiers").str());
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

// Outputs the contents of the render queue to debug-out
void CoreEngine::DebugOutputRenderQueueContents(void)
{
	std::ostringstream ss;
	ss << "Render Queue Shaders\n" <<
		"{\n";

	for (int i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		ss << "\tShader " << i << " [AlphaBlend=" << m_renderqueueshaders[i].AlphaBlendRequired << ", ZSort=" << m_renderqueueshaders[i].RequiresZSorting <<
			", Topology=" << m_renderqueueshaders[i].PrimitiveTopology << "]\n";
	}
	ss << "}\n";

	OutputDebugString(ss.str().c_str());
}

// Default destructor
CoreEngine::~CoreEngine(void)
{
}

