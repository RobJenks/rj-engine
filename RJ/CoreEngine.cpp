#include "DX11_Compatibility.h"		// Should be included first when performing compatibility checks
#include "DX11_Core.h" 

#include "ErrorCodes.h"
#include "GlobalFlags.h"
#include "Logging.h"
#include "RJMain.h"
#include "DeferredRenderProcess.h"
#include "SDFDecalRenderProcess.h"
#include "UIRenderProcess.h"
#include "Profiler.h"
#include "FrameProfiler.h"
#include "Timers.h"
#include "CameraClass.h"
#include "TextureDX11.h"
#include "InputLayoutDesc.h"
#include "LightingManagerObject.h"
#include "DecalRenderingManager.h"
#include "ShaderFlags.h"
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
#include "Frustum.h"
#include "BoundingObject.h"
#include "MaterialDX11.h"
#include "PipelineStateDX11.h"
#include "FontShader.h"
#include "AudioManager.h"
#include "TextRenderer.h"
#include "EffectManager.h"
#include "ParticleEngine.h"
#include "Render2DManager.h"
#include "SkinnedNormalMapShader.h"
#include "VolLineShader.h"
#include "OverlayRenderer.h"
#include "NoiseGenerator.h"
#include "BasicColourDefinition.h"

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
#include "TerrainDefinition.h"
#include "Actor.h"
#include "GameConsoleCommand.h"
#include "VolumetricLine.h"
#include "EnvironmentTree.h"
#include "NavNetwork.h"
#include "NavNode.h"
#include "SentenceType.h"
#include "PortalRenderingStep.h"
#include "DynamicTerrain.h"						// DBG
#include "DynamicTerrainDefinition.h"			// DBG
#include <tchar.h>
#include <unordered_map>
#include "CoreEngine.h"

// Forward declare allowed instances of render queue processing
template void CoreEngine::ProcessRenderQueue<ShaderRenderPredicate::RenderGeometry, ModelRenderPredicate::RenderAll>(PipelineStateDX11*);
template void CoreEngine::ProcessRenderQueue<ShaderRenderPredicate::RenderGeometry, ModelRenderPredicate::RenderNonTransparent>(PipelineStateDX11*);

template void CoreEngine::ProcessRenderQueue<ShaderRenderPredicate::RenderUI, ModelRenderPredicate::RenderAll>(PipelineStateDX11*);

// Default constructor
CoreEngine::CoreEngine(void)
	:
	m_renderdevice(NULL),
	m_rq_optimiser(m_renderqueue), 
	m_camera(NULL),
	LightingManager(NULL), 
	m_lightshader(NULL),
	m_lightfadeshader(NULL),
	m_lighthighlightshader(NULL),
	m_lighthighlightfadeshader(NULL),
	m_particleshader(NULL),
	m_textureshader(NULL),
	m_texcubeshader(NULL),
	m_frustrum(NULL),
	m_decalrenderer(NULL), 
	m_textrenderer(NULL),
	m_fontshader(NULL),
	m_fireshader(NULL),
	m_skinnedshader(NULL),
	m_vollineshader(NULL),
	m_audiomanager(NULL), 
	m_effectmanager(NULL),
	m_particleengine(NULL),
	m_render2d(NULL),
	m_overlayrenderer(NULL),
	m_noisegenerator(NULL), 
	m_instancebuffer(NULL),
	m_current_topology( D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED ), 
	m_hwnd( NULL ),
	m_vsync( false ), 
	m_render_device_failure_count(0U), 
	m_screen_space_adjustment(NULL_VECTOR), 
	m_screen_space_adjustment_f(NULL_FLOAT2), 
	m_screenspace_quad_vb(NULL)
{
	// Reset all debug component pointers
	m_debug_renderenvboxes = m_debug_renderenvtree = m_debug_renderportaltraversal = 0;
	m_debug_portal_debugrender = m_debug_portal_debuglog = false;
	m_debug_portal_render_initial_frustum = NULL;
	m_debug_renderobjid_object = 0;
	m_debug_renderobjid_distance = 1000.0f;
	m_debug_terrain_render_mode = DebugTerrainRenderMode::Normal;

	// Initialise all render stage flags to true at startup
	m_renderstages = std::vector<bool>(CoreEngine::RenderStage::Render_STAGECOUNT, true);
	
	// Initialise all special render flags at startup
	m_renderflags = std::vector<bool>(CoreEngine::RenderFlag::_RFLAG_COUNT, false);

	// Initialise miscellaneous cached data
	m_instanceparams = NULL_FLOAT4;
	m_unit_quad_model = NULL;

	// Initialise all key matrices to the identity
	r_view = r_projection = r_orthographic = r_invview = r_invproj = r_invorthographic = r_viewproj = r_invviewproj = m_projscreen
		= r_viewprojscreen = r_invviewprojscreen = r_projection_unjittered = r_viewproj_unjittered = ID_MATRIX;
	r_view_f = r_projection_f = r_orthographic_f = r_invview_f = r_invproj_f = r_invorthographic_f = r_viewproj_f 
		= r_invviewproj_f = r_projection_unjittered_f = r_viewproj_unjittered_f = r_priorframe_viewproj_f = r_priorframe_viewproj_unjittered_f = ID_MATRIX_F;
	
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

	m_hwnd = hwnd;

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

	// Initialise shader support data
	res = InitialiseShaderSupport();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Shader support data initialised\n";

	// Initialise the render device component
	res = InitialiseRenderDevice(hwnd);
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Render device initialisation complete\n";

	// Initialise the camera component
	res = InitialiseCamera();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Camera initialised\n";

	// Initialise the view frustrum
	res = InitialiseFrustrum();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "View frustum created\n";

	// Initialise the audio manager
	res = InitialiseAudioManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Audio manager initialisation complete\n";

	// Initialise the lighting manager
	res = InitialiseLightingManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Lighting manager initialisation complete\n";

	// Initialise the decal render manager
	res = InitialiseDecalRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Decal renderer initialisation complete\n";

	// Initialise the text rendering components
	res = InitialiseTextRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Text rendering initialised\n";

	// Initialise the specialised screen-space rendering components
	res = InitialiseScreenSpaceRenderingComponents();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Screen-space rendering components initialised\n";

	// Initialise the effect manager
	res = InitialiseEffectManager();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Effect manager initialised\n";

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
	
	// Initialise the rendering noise generator
	res = InitialiseNoiseGenerator();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Noise generator initialised\n";

	// Initialise the render queue for geometry instancing & batching (dependent on initialisation of relevant shaders)
	res = InitialiseRenderQueue();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Render queue initialised for all shaders\n";

	// Initialise the components used for environment rendering
	res = InitialiseEnvironmentRendering();
	if (res != ErrorCodes::NoError) { ShutdownGameEngine(); return res; }
	Game::Log << LOG_INFO << "Environment rendering initialised\n";

	// Activate the required render processes for this configuration
	Game::Engine->GetRenderDevice()->ActivateRenderProcess<DeferredRenderProcess>(RenderProcess::RenderProcessClass::Primary);
	Game::Engine->GetRenderDevice()->ActivateRenderProcess<SDFDecalRenderProcess>(RenderProcess::RenderProcessClass::Decal);
	Game::Engine->GetRenderDevice()->ActivateRenderProcess<UIRenderProcess>(RenderProcess::RenderProcessClass::UI);

	// If we succeed in all initialisation functions then return success now
	Game::Log << LOG_INFO << "All game engine initialisation completed successfully\n\n";
	return ErrorCodes::NoError;
}

// Perform any post-data-load activities, e.g.retrieving models that have now been loaded
Result CoreEngine::PerformPostDataLoadInitialisation(void)
{
	// Perform post-load initialisation for the core engine itself
	PerformInternalEnginePostDataLoadInitialisation();

	// Perform post-load initialisation for all sub-components which require it
	GetRenderDevice()->PerformPostDataLoadInitialisation();
	GetEffectManager()->PerformPostDataLoadInitialisation();
	GetOverlayRenderer()->PerformPostDataLoadInitialisation();

	return ErrorCodes::NoError;
}

// Post-data load initialisation for the core engine component itself
Result CoreEngine::PerformInternalEnginePostDataLoadInitialisation(void)
{
	Result result = ErrorCodes::NoError;

	// Load all required model geometry
	std::vector<std::tuple<std::string, std::string, Model**>> models = {
		{ "unit quad", "unit_square_model", &m_unit_quad_model }
	};

	for (auto & model : models)
	{
		Model *m = Model::GetModel(std::get<1>(model));
		if (!m)
		{
			Game::Log << LOG_ERROR << "Could not load " << std::get<0>(model) << " model (\"" << std::get<1>(model) << "\") during deferred render process initialisation\n";
			result = ErrorCodes::ModelDoesNotExist;
		}

		*(std::get<2>(model)) = m;
	}

	return result;
}

void CoreEngine::ShutdownGameEngine()
{
	// Run the termination function for each component
	ShutdownCamera();
	ShutdownShaderSupport();
	ShutdownFrustrum();
	ShutdownAudioManager();
	ShutdownLightingManager();
	ShutdownDecalRendering();
	ShutdownTextRendering();
	ShutdownScreenSpaceRenderingComponents();
	ShutdownEffectManager();
	ShutdownParticleEngine();
	Shutdown2DRenderManager();
	ShutdownOverlayRenderer();
	ShutdownNoiseGenerator();
	ShutdownRenderQueue();
	ShutdownEnvironmentRendering();
	ShutdownTextureData();
	ShutdownRenderDevice();
}

Result CoreEngine::InitialiseRenderFlags(void)
{
	// Initialise all render flags to default values
	
	// Return success
	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseRenderDevice(HWND hwnd)
{
	// Attempt to create the render device object.
	m_renderdevice = new RenderDeviceDX11();
	if ( !m_renderdevice ) return ErrorCodes::CannotCreateRenderDevice;

	// Perform all render engine initialisation
	Result result = m_renderdevice->Initialise(	hwnd, INTVECTOR2(Game::ScreenWidth, Game::ScreenHeight), Game::FullScreen, 
												Game::VSync, Game::NearClipPlane, Game::FarClipPlane);
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
	// Create the instance buffer that will be reused by the render queue for all rendering
	// This must be a DYNAMIC vertex buffer since we will be re-mapping instance data multiple times every frame
	m_instancebuffer = GetRenderDevice()->Assets.CreateVertexBuffer<RM_Instance>("InstanceBuffer", static_cast<UINT>(Game::C_INSTANCED_RENDER_LIMIT), true);

	// Initialise the buffer pointers, stride and offset values
	m_instancedbuffers[0] = NULL;									// Buffer[0] will be populated with each VB
	m_instancedbuffers[1] = m_instancebuffer->GetCompiledBuffer();	// Buffer[1] is the instance buffer
	m_instancedstride[0] = 0U;										// Stride[0] will be populated with the model-specific vertex size
	m_instancedstride[1] = sizeof(RM_Instance);						// Buffer[1] is the instance buffer
	m_instancedoffset[0] = 0; m_instancedoffset[1] = 0;				// No offsets in either buffer

	// Initialise the render queue with a blank map for each shader in scope
	m_renderqueue = RenderQueue(RenderQueueShader::RM_RENDERQUEUESHADERCOUNT);
	m_renderqueueshaders = RM_ShaderCollection(RenderQueueShader::RM_RENDERQUEUESHADERCOUNT);

	// Set the reference and parameters for each shader in turn
	m_renderqueueshaders[RenderQueueShader::RM_LightShader] = 
		RM_InstancedShaderDetails((iShader*)m_lightshader, false, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (ShaderFlags)ShaderFlag::ShaderTypeGeometry);

	m_renderqueueshaders[RenderQueueShader::RM_OrthographicTexture] =
		RM_InstancedShaderDetails((iShader*)NULL, false, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (ShaderFlags)ShaderFlag::ShaderTypeUI);

	m_renderqueueshaders[RenderQueueShader::RM_LightHighlightShader] = 
		RM_InstancedShaderDetails((iShader*)m_lighthighlightshader, false, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (ShaderFlags)ShaderFlag::ShaderTypeGeometry);
	m_renderqueueshaders[RenderQueueShader::RM_LightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lightfadeshader, true, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (ShaderFlags)ShaderFlag::ShaderTypeGeometry);
	m_renderqueueshaders[RenderQueueShader::RM_LightHighlightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lighthighlightfadeshader, true, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (ShaderFlags)ShaderFlag::ShaderTypeGeometry);
	m_renderqueueshaders[RenderQueueShader::RM_LightFlatHighlightFadeShader] =
		RM_InstancedShaderDetails((iShader*)m_lightflathighlightfadeshader, true, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (ShaderFlags)ShaderFlag::ShaderTypeGeometry);
	m_renderqueueshaders[RenderQueueShader::RM_VolLineShader] =
		RM_InstancedShaderDetails((iShader*)m_vollineshader, true, D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST, (ShaderFlags)ShaderFlag::ShaderTypeGeometry);

	// TODO: DEBUG: Remove variants on the light shader
	/*m_renderqueueshaders[RenderQueueShader::RM_LightHighlightShader] = m_renderqueueshaders[RenderQueueShader::RM_LightShader];
	m_renderqueueshaders[RenderQueueShader::RM_LightFadeShader] = m_renderqueueshaders[RenderQueueShader::RM_LightShader];
	m_renderqueueshaders[RenderQueueShader::RM_LightHighlightFadeShader] = m_renderqueueshaders[RenderQueueShader::RM_LightShader];*/

	// Initialise render queue slots with this data
	// TODO: a little redundant to have both...?
	for (int i = 0; i < (int)RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		m_renderqueue[i].SetShaderDetails(m_renderqueueshaders[i]);
	}


	// Set an initial primitive topology as a default
	// TODO: Temporary fix for actual problem: if RenderInstanced was called before ProcessRenderQueue, no topology had been set and 
	// engine failed with D3D exception.  In general, should not rely on this being set only by render queue.  Should perhaps be set
	// as part of pipeline binding?
	r_devicecontext = m_renderdevice->GetDeviceContext();
	ChangePrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
	XMMATRIX not_required;
	m_camera->CalculateViewMatrixFromPositionData(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), ID_QUATERNION, m_baseviewmatrix, not_required);
	
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

Result CoreEngine::InitialiseScreenSpaceRenderingComponents(void)
{
	// Directly-populated vertex buffer which matches the minimal VS input layout
	XMFLOAT2 verts[6] = { { -1.0f, -1.0f }, { -1.0f, +1.0f }, { +1.0f, +1.0f }, { -1.0f, -1.0f }, { +1.0f, +1.0f }, { +1.0f, -1.0f } };
	D3D11_BUFFER_DESC desc = { 6U * sizeof(XMFLOAT2), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
	D3D11_SUBRESOURCE_DATA data = { (void *)verts, sizeof(XMFLOAT2), 6U * sizeof(XMFLOAT2) };
	HRESULT hr = GetDevice()->CreateBuffer(&desc, &data, &m_screenspace_quad_vb);

	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Failed to initialised screen-space rendering support components\n";
		return ErrorCodes::CannotInitialiseScreenSpaceRenderingSupport;
	}

	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseFrustrum()
{
	// Attempt to create a new view frustrum
	m_frustrum = new Frustum(4U);
	if (!m_frustrum) return ErrorCodes::CannotCreateViewFrustrum;
	
	// Run the initialisation function with viewport/projection data that can be precaulcated
	Result res = m_frustrum->InitialiseAsViewFrustum(m_renderdevice->GetProjectionMatrix(), Game::FarClipPlane, m_renderdevice->GetFOV(), m_renderdevice->GetAspectRatio());
	if (res != ErrorCodes::NoError) return res;	

	// Return success if the frustrum was created
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

Result CoreEngine::InitialiseLightingManager(void)
{
	LightingManager = new LightingManagerObject();
	if (!LightingManager)
	{
		return ErrorCodes::CannotInitialiseLightingManager;
	}

	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseDecalRendering(void)
{
	m_decalrenderer = new DecalRenderingManager();
	if (!m_decalrenderer)
	{
		return ErrorCodes::CannotInitialiseDecalRenderer;
	}

	return ErrorCodes::NoError;
}

Result CoreEngine::InitialiseTextRendering(void)
{
	m_textrenderer = new TextRenderer();
	if (!m_textrenderer)
	{
		return ErrorCodes::CannotCreateTextRenderer;
	}

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
	result = m_effectmanager->Initialise(GetDevice());
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

Result CoreEngine::InitialiseParticleEngine(void)
{
	Result result;

	// Create the particle engine object
	m_particleengine = new ParticleEngine();
	if (!m_particleengine) return ErrorCodes::CouldNotCreateParticleEngine;

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
	Result result = m_render2d->Initialise(GetDevice(), GetDeviceContext(), m_hwnd, 
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

Result CoreEngine::InitialiseNoiseGenerator(void)
{
	m_noisegenerator = new NoiseGenerator();

	Result result = m_noisegenerator->Initialise();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

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


void CoreEngine::ShutdownRenderDevice(void)
{
	// Render engine destructor will release all related resources and centrally-maintained 
	// components (shaders, states, buffers).  Should therefore be terminated late
	if ( m_renderdevice )
	{
		SafeDelete(m_renderdevice);
	}
}

void CoreEngine::ShutdownRenderQueue(void)
{
	// Free all resources that are not automatically deallocated
}

void CoreEngine::ShutdownTextureData(void)
{	
	// Shutdown all texture data in the static global collection
	TextureDX11::ShutdownGlobalTextureCollection();
}

void CoreEngine::ShutdownCamera(void)
{
	// Attempt to release the camera component
	if ( m_camera )
	{
		SafeDelete(m_camera);
	}
}

void CoreEngine::ShutdownShaderSupport(void)
{
	// Nothing required
}

void CoreEngine::ShutdownScreenSpaceRenderingComponents(void)
{
	// Release any specific resources held for screen-space rendering
	ReleaseIfExists(m_screenspace_quad_vb);
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

void CoreEngine::ShutdownLightingManager(void)
{
	// Release the lighting manager object
	if (LightingManager)
	{
		SafeDelete(LightingManager);
	}
}

void CoreEngine::ShutdownDecalRendering(void)
{
	// Release the decal rendering manager
	if (m_decalrenderer)
	{
		SafeDelete(m_decalrenderer);
	}
}

void CoreEngine::ShutdownTextRendering(void)
{
	// Release the text manager object.
	if (m_textrenderer)
	{
		m_textrenderer->Shutdown();
		SafeDelete(m_textrenderer);
	}
}

void CoreEngine::ShutdownEffectManager(void)
{
	if (m_effectmanager)
	{
		m_effectmanager->Shutdown();
		SafeDelete(m_effectmanager);
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

void CoreEngine::ShutdownNoiseGenerator(void)
{
	if (m_noisegenerator)
	{
		SafeDelete(m_noisegenerator);
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

	// Recalculate the screen-space adjustment which is dependent on screen dimensions
	RecalculateScreenSpaceAdjustment();
}

// Set the visibility state of the system cursor
void CoreEngine::SetSystemCursorVisibility(bool cursor_visible)
{
	ShowCursor(cursor_visible ? TRUE : FALSE);
}

// Pre-frame initialisation for the engine and its components
void CoreEngine::BeginFrame(void)
{
	// Delegate to engine components as required
	GetRenderDevice()->BeginFrame();
	GetDecalRenderer()->BeginFrame();
	GetNoiseGenerator()->BeginFrame();
}

// Pre-frame tear-down for the engine and its components
void CoreEngine::EndFrame(void)
{
	// Delegate to engine components as required
	GetDecalRenderer()->EndFrame();
}

// The main rendering function; renders everything in turn as required
void CoreEngine::Render(void)
{
	// Reset render statistics ready for the next frame
	ResetRenderInfo();

	// Retrieve render-cycle-specific data that will not change for the duration of the cycle.  Prefixed r_*
	RetrieveRenderCycleData();
	
	// Verify the render device is in a good state and report errors if not
	if (GetRenderDevice()->VerifyState() == false)
	{
		++m_render_device_failure_count;
		Game::Log << LOG_ERROR << "SEVERE: Render device failed state verification; frame skipped.  Failure " << m_render_device_failure_count << " of "
			<< ALLOWABLE_RENDER_DEVICE_FAILURE_COUNT << " allowable failure cases\n";
		if (m_render_device_failure_count > ALLOWABLE_RENDER_DEVICE_FAILURE_COUNT)
		{
			Game::Log << LOG_ERROR << "FATAL: Render device failures exceed allowable threshold; invoking application shutdown\n";
			Game::Application.Quit();
		}
	}

	// Run any pre-render debug processes
#	ifdef _DEBUG
		RunPreRenderDebugProcesses();
#	endif

	// Construct the view frustrum for this frame so we can perform culling calculations
	m_frustrum->ConstructViewFrustrum(r_view, r_invview);

	// Determine the lighting configuration visible in the current frame
	LightingManager->AnalyseNewFrame();

	/* Process the scene and populate the render queue (TODO: break out and add remainder; actors, particles, ...) */
	SpaceSystem & system = Game::Universe->GetCurrentSystem();
	RenderAllSystemObjects(system);

	/* Render all user interface components */
	RenderUserInterface();

	/* Perform debug rendering */
	RenderDebugData();

	/* Invoke the active render process which will orchestrate all rendering activities for the frame */
	m_renderdevice->Render();

	/* Present the scene once all rendering is complete */
	m_renderdevice->PresentFrame();

	// Activate the render queue optimiser here if it is ready for its next cycle, then clear the render queue ready for Frame+1
	if (m_rq_optimiser.Ready()) m_rq_optimiser.Run();
	ClearRenderQueue();
	
	// Run any post-render debug processes
#	ifdef _DEBUG
		RunPostRenderDebugProcesses();
#	endif

	// End the frame
	
}

// Retrieve render-cycle-specific data that will not change for the duration of the cycle.  Prefixed r_*
void CoreEngine::RetrieveRenderCycleData(void)
{
	// Store prior-frame data before recalculating
	// TODO: Define render matrices within struct, then maintain a "Current" and "PriorFrame" instance
	r_priorframe_viewproj_f = r_viewproj_f;
	r_priorframe_viewproj_unjittered_f = r_viewproj_unjittered_f;

	// Store new render matrices
	r_devicecontext = m_renderdevice->GetDeviceContext();
	m_camera->GetViewMatrix(r_view);
	m_camera->GetInverseViewMatrix(r_invview);
	r_projection = m_renderdevice->GetProjectionMatrix();
	r_projection_unjittered = m_renderdevice->GetProjectionMatrixUnjittered();
	r_invproj = m_renderdevice->GetInverseProjectionMatrix();
	r_orthographic = m_renderdevice->GetOrthoMatrix();
	r_invorthographic = XMMatrixInverse(NULL, r_orthographic);
	r_viewproj = XMMatrixMultiply(r_view, r_projection);
	r_viewproj_unjittered = XMMatrixMultiply(r_view, r_projection_unjittered);
	r_invviewproj = XMMatrixMultiply(r_invproj, r_invview);
	r_viewprojscreen = XMMatrixMultiply(r_viewproj, m_projscreen);
	r_invviewprojscreen = XMMatrixInverse(NULL, r_viewprojscreen);

	// Store local float representations of each key matrix for runtime efficiency
	XMStoreFloat4x4(&r_view_f, r_view);
	XMStoreFloat4x4(&r_invview_f, r_invview);
	XMStoreFloat4x4(&r_projection_f, r_projection);
	XMStoreFloat4x4(&r_projection_unjittered_f, r_projection_unjittered);
	XMStoreFloat4x4(&r_invproj_f, r_invproj);
	XMStoreFloat4x4(&r_orthographic_f, r_orthographic);
	XMStoreFloat4x4(&r_invorthographic_f, r_invorthographic);
	XMStoreFloat4x4(&r_viewproj_f, r_viewproj);
	XMStoreFloat4x4(&r_viewproj_unjittered_f, r_viewproj_unjittered);
	XMStoreFloat4x4(&r_invviewproj_f, r_invviewproj);
}

// Submits a model to the render queue manager for rendering this frame.  Will iterate through all model components
// and dispatch to the render queue individually
void RJ_XM_CALLCONV	CoreEngine::SubmitForRendering(RenderQueueShader shader, Model *model, MaterialDX11 *material, RM_Instance && instance, RM_InstanceMetadata && metadata)
{
	if (!model) return;

	// Single-component optimisation (TODO: probably; unless branch cost > loop & instance copy-cons cost)
	size_t n = model->GetComponentCount();
	if (n == 1U)
	{
		SubmitForRendering(shader, (model->Components[0].Data.get()), material, std::move(instance), std::move(metadata));
		return;
	}

	// Otherwise process each component in turn
	// TODO: optimisation; loop through 1 to N-1, then std::move() the actual instance data for [0] directly.  Saves copying once
	for (size_t i = 0U; i < n; ++i)
	{
		SubmitForRendering(shader, (model->Components[i].Data.get()), material, std::move(RM_Instance(instance)), std::move(RM_InstanceMetadata(metadata)));
	}
}

// Submit a model buffer to the render queue manager for rendering this frame.  A material can be supplied that
// overrides any default material specified in the model buffer.  A material of NULL will use the default 
// material in the model buffer
void RJ_XM_CALLCONV CoreEngine::SubmitForRendering(RenderQueueShader shader, ModelBuffer *model, MaterialDX11 *material, RM_Instance && instance, RM_InstanceMetadata && metadata)
{
	// Exclude any null-geometry objects
	if (!model) return;

	// Retrieve the most appropriate render slot and move this instance into it
	size_t render_slot = model->GetAssignedRenderSlot(shader);
	if (render_slot == ModelBuffer::NO_RENDER_SLOT)
	{
		render_slot = m_renderqueue[shader].NewRenderSlot();
		m_renderqueue.RegisterModelBuffer(shader, render_slot, model);
	}

	m_renderqueue[shader].ModelData[render_slot].GetInstances(material).NewInstance(std::move(instance), std::move(metadata));
}

// Submit a model for z-sorted rendering.  Will dispatch each model component to the render queue in turn
void RJ_XM_CALLCONV CoreEngine::SubmitForZSortedRendering(RenderQueueShader shader, Model *model, RM_Instance && instance, const CXMVECTOR position)
{
	if (!model) return;

	// Single-component optimisation (TODO: probably; unless branch cost > loop & instance copy-cons cost)
	size_t n = model->GetComponentCount();
	if (n == 1U)
	{
		SubmitForZSortedRendering(shader, (model->Components[0].Data.get()), std::move(instance), position);
		return;
	}

	// Otherwise process each component in turn
	for (size_t i = 0U; i < n; ++i)
	{
		SubmitForZSortedRendering(shader, (model->Components[i].Data.get()), std::move(RM_Instance(instance)), position);
	}
}


// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
// objects.  Performance overhead; should be used only where specifically required
void RJ_XM_CALLCONV CoreEngine::SubmitForZSortedRendering(RenderQueueShader shader, ModelBuffer *model, RM_Instance && instance, CXMVECTOR position)
{
	// Exclude any null-geometry objects
	if (!model || model->VertexBuffer.GetCompiledBuffer() == NULL) return;

	// Determine the distance to this object so we can use it as a sort key
	int z = (int)XMVectorGetX(XMVector3LengthSq(position - m_camera->GetPosition()));

	// Add to the z-sorted vector with this z-value as the sorting key
	m_renderqueueshaders[shader].SortedInstances.push_back(std::move(RM_ZSortedInstance(z, model, std::move(instance))));
}

// Submit a material directly for orthographic rendering (of its diffuse texture) to the screen
void RJ_XM_CALLCONV CoreEngine::RenderMaterialToScreen(MaterialDX11 & material, const XMFLOAT2 & position, const XMFLOAT2 size, float rotation, float opacity, float zorder)
{
	// Transform location to desired linear screen-space.  Specified as position of centre of object being rendered
	XMFLOAT2 adjusted_pos = AdjustIntoLinearScreenSpace(position, size);

	// Build a transform matrix based on the given screen-space properties
	XMMATRIX transform = XMMatrixMultiply(XMMatrixMultiply(
		XMMatrixScaling(size.x, size.y, 1.0f),
		XMMatrixRotationZ(rotation)),
		XMMatrixTranslation(adjusted_pos.x, adjusted_pos.y, zorder));

	// Delegate to the primary submission method
	SubmitForRendering(RenderQueueShader::RM_OrthographicTexture, m_unit_quad_model->Components[0].Data.get(), &material,
		std::move(RM_Instance(transform, RM_Instance::SORT_KEY_RENDER_FIRST, XMFLOAT4(opacity, 0.0f, 0.0f, 0.0f), InstanceFlags::DEFAULT_INSTANCE_FLAGS)), 
		std::move(RM_InstanceMetadata())
	);
}

// Adjust the given screen location to desired screen-space reference frame with (0,0) in the top-left of the screen
XMVECTOR CoreEngine::AdjustIntoLinearScreenSpaceCentred(const FXMVECTOR location)
{
	const XMVECTOR & adjust = ScreenSpaceAdjustment();
	return XMVectorSelect(XMVectorAdd(adjust, location), XMVectorSubtract(adjust, location), g_XMSelect0101);	// Need 01__ so this is fine
}

// Adjust the given screen location to desired screen-space reference frame with (0,0) in the top-left of the screen
XMFLOAT2 CoreEngine::AdjustIntoLinearScreenSpaceCentred(XMFLOAT2 location)
{
	const XMFLOAT2 & adjust = ScreenSpaceAdjustmentF();
	return XMFLOAT2(adjust.x + location.x, adjust.y - location.y);
}

// Adjust the given screen location to desired screen-space reference frame with (0,0) in the top-left of the screen, and coords
// expressed relative to the top-left of the object being placed
XMVECTOR CoreEngine::AdjustIntoLinearScreenSpace(const FXMVECTOR location, const FXMVECTOR size)
{
	const XMVECTOR & adjust = ScreenSpaceAdjustment();
	XMVECTOR centrepos = XMVectorMultiplyAdd(size, HALF_VECTOR, location);										// Location + (Size * 0.5)
	return XMVectorSelect(XMVectorAdd(adjust, centrepos), XMVectorSubtract(adjust, centrepos), g_XMSelect0101);	// Need 01__ so this is fine
}

// Adjust the given screen location to desired screen-space reference frame with (0,0) in the top-left of the screen, and coords
// expressed relative to the top-left of the object being placed
XMFLOAT2 CoreEngine::AdjustIntoLinearScreenSpace(XMFLOAT2 location, XMFLOAT2 size)
{
	const XMFLOAT2 & adjust = ScreenSpaceAdjustmentF();
	return XMFLOAT2(adjust.x + location.x + (size.x * 0.5f), adjust.y - (location.y + (size.y * 0.5f)));
}

// Process all items in the queue via instanced rendering.  All instances for models passing the supplied render predicates
// will be rendered through the given rendering pipeline
template <class TShaderRenderPredicate, class TModelRenderPredicate>
void CoreEngine::ProcessRenderQueue(PipelineStateDX11 *pipeline)
{
	size_t model_count, material_count, instancecount, inst;
	UINT batch_size;
	TShaderRenderPredicate render_shader;
	TModelRenderPredicate render_model;
	ModelBuffer * modelbuffer;

	if (!pipeline) return;

	// Iterate through each shader in the render queue (though not currently required; all will be mapped to 0)
	for (auto i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		// Verfy against the shader rendering predicate before proceeding
		RM_ModelDataCollection & rq_shader = m_renderqueue[i];
		if (!render_shader(rq_shader)) continue;

		// Early-exit for empty shader queues, before changing any state below
		model_count = rq_shader.CurrentSlotCount;
		if (model_count == 0U) continue;

		// Set the type of primitive that should be rendered through this shader, if it needs to be changed
		ChangePrimitiveTopologyIfRequired(rq_shader.PrimitiveTopology);

		// Process each model separately
		for (size_t mi = 0U; mi < model_count; ++mi)
		{
			// Early-exit if this model has no compiled geometry or fails the model rendering predicate
			RM_ModelData & model = rq_shader.ModelData[mi];
			modelbuffer = model.ModelBufferInstance;
			if (!modelbuffer || !modelbuffer->VertexBuffer.GetCompiledBuffer()) continue;
			if (!render_model(modelbuffer)) continue;

			// Process each material that will be used to render the instances
			material_count = model.CurrentMaterialCount;
			for (size_t mat = 0U; mat < material_count; ++mat)
			{
				auto & model_data = model.Data[mat];

				/// TODO: Sort instances here?

				// Loop through the instances in batches, if the total count is larger than our limit
				instancecount = model_data.InstanceCollection.CurrentInstanceCount;
				for (inst = 0U; inst < instancecount; inst += Game::C_INSTANCED_RENDER_LIMIT)
				{
					// Determine the number of instances to render; either the per-batch limit, or fewer if we do not have that many
					batch_size = static_cast<UINT>(min(instancecount - inst, Game::C_INSTANCED_RENDER_LIMIT));

					// Pass control to the core instanced rendering method to issue a draw call
					RenderInstanced(*pipeline, *modelbuffer, model_data.Material, model_data.InstanceCollection.InstanceData[inst], batch_size);

				} /// per-instance

			} /// per-material

		} /// per-model

	} /// per-shader

}

// Perform instanced rendering for a model and a set of instance data; generally called by the render queue but can be 
// invoked by other processes (e.g. for deferred light volume rendering).  A material can be supplied that will override
// the material specified in the model buffer; a null material will fall back to the default model buffer material
void CoreEngine::RenderInstanced(const PipelineStateDX11 & pipeline, const Model & model, const MaterialDX11 * material, const RM_Instance & instance_data, UINT instance_count)
{
	for (const auto & component : model.Components)
	{
		const auto buffer = component.Data.get();
		if (!buffer) continue;							// TODO: If we can guarantee that Models will only ever contain valid components, we can avoid checks like this during rendering

		RenderInstanced(pipeline, *buffer, material, instance_data, instance_count);
	}
}

// Perform instanced rendering for a model and a set of instance data; generally called by the render queue but can be 
// invoked by other processes (e.g. for deferred light volume rendering).  A material can be supplied that will override
// the material specified in the model buffer; a null material will fall back to the default model buffer material
void CoreEngine::RenderInstanced(const PipelineStateDX11 & pipeline, const ModelBuffer & model, const MaterialDX11 * material, const RM_Instance & instance_data, UINT instance_count)
{
	// Update the instance buffer by mapping, updating and unmapping the memory
	m_instancebuffer->Set(&instance_data, static_cast<UINT>(sizeof(RM_Instance)) * instance_count);

	// The render queue will take ownership for binding vertex buffers ({ vertices, instances}) so 
	// that we can bind both to the shader in parallel.  m_instancedbuffers[1] = instancebuffer, so just 
	// set [0] before binding
	m_instancedbuffers[0] = model.VertexBuffer.GetCompiledBuffer();
	m_instancedstride[0] = model.VertexBuffer.GetStride();
	r_devicecontext->IASetVertexBuffers(0, 2, (ID3D11Buffer * const *)(&m_instancedbuffers[0]), m_instancedstride, m_instancedoffset);

	// Set the model index buffer to active in the input assembler
	r_devicecontext->IASetIndexBuffer(model.IndexBuffer.GetCompiledBuffer(), IndexBufferDX11::INDEX_FORMAT, 0U);

	// Bind the model material, which will populate the relevant constant buffer and bind all required texture resources
	// TODO: For now, bind explicitly to the VS and PS.  In future we may want to add more, or make this specifiable per shader
	// Use override material if available, otherwise fall back to the model buffer material (which may also be null in [error] cases)
	// Deliberate fallthrough from (!x) to (x) since model.Material may also be NULL
	// TODO (SM): don't bind materials to all shaders; only when shader e.g. exposes a 'uses_materials' flag
	if (!material) material = model.Material;
	if (material)
	{
		material->Bind(pipeline.GetShader(Shader::Type::VertexShader));
		material->Bind(pipeline.GetShader(Shader::Type::PixelShader));
	}

	// Issue a draw call to the currently active render pipeline
	r_devicecontext->DrawIndexedInstanced(model.IndexBuffer.GetIndexCount(), instance_count, 0U, 0, 0);

	// Update the total count of draw calls & instances that have been processed
	++m_renderinfo.DrawCalls; 
	m_renderinfo.InstanceCount += instance_count;
}

// Specialised method for full-screen quad rendering, to support cheaper post-processing and screen-space rendering
// Is not processed through the render queue; these render actions are performed immediately
void CoreEngine::RenderFullScreenQuad(void)
{
	const UINT stride = sizeof(XMFLOAT2);
	const UINT offset = 0U;

	// Bind the precompiled fullscreen rendering VB
	r_devicecontext->IASetVertexBuffers(0, 1, (ID3D11Buffer * const *)(&m_screenspace_quad_vb), &stride, &offset);

	// Submit a basic draw call for the quad geometry
	r_devicecontext->Draw(6U, 0U);
}

// Clear the render queue.  No longer performed during render queue processing since we need to be able to process all render
// queue items multiple times through e.g. different shader pipelines
void CoreEngine::ClearRenderQueue(void)
{
	// For each shader in the render queue
	for (size_t i = 0U; i < (size_t)RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		auto & modelqueue = m_renderqueue[i];
		auto model_count = modelqueue.CurrentSlotCount;

		// For each model being rendered by this shader
		for (size_t mi = 0U; mi < model_count; ++mi)
		{
			// Unregister this model from the given render queue slot
			m_renderqueue.UnregisterModelBuffer(i, mi);

			// Reset the per-material data back to default state
			m_renderqueue[i].ModelData[mi].Reset();
		}

		// Reset this render queue shader ready for the next frame
		m_renderqueue[i].CurrentSlotCount = 0U;
	}
}

void CoreEngine::ChangePrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitive_topology)
{
	r_devicecontext->IASetPrimitiveTopology(primitive_topology);
	m_current_topology = primitive_topology;
}

void CoreEngine::ChangePrimitiveTopologyIfRequired(D3D_PRIMITIVE_TOPOLOGY primitive_topology)
{
	if (primitive_topology != GetCurrentPrimitiveTopology())
	{
		ChangePrimitiveTopology(primitive_topology);
	}
}

// Processes all items in the render queue using instanced rendering, to minimise the number of render calls required per frame
/*RJ_PROFILED(void CoreEngine::ProcessRenderQueueOld, void)
{
	D3D11_MAPPED_SUBRESOURCE mappedres;
	std::vector<RM_Instance>::size_type instancecount, inst, n;
#	ifdef RJ_ENABLE_FRAME_PROFILER
	Timers::HRClockTime render_time;
#	endif

	// Debug output of render queue contents, if applicable
	RJ_FRAME_PROFILER_EXECUTE(DebugOutputRenderQueueContents();)

	// Iterate through each shader in the render queue
	for (auto i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		// Get a reference to this specific render queue 
		RM_InstancedShaderDetails & rq_shader = m_renderqueueshaders[i];
		RJ_FRAME_PROFILER_OUTPUT(concat("Activating shader ")(i)(" [")
			((rq_shader.RequiresZSorting ? rq_shader.SortedInstances.size() : m_renderqueue[i].CurrentSlotCount))(" models]\n").str().c_str())

		// Skip this shader immediately if there are no models/instances to be rendered by it (different check depending on whether this is a z-sorted shader)
		auto model_count = m_renderqueue[i].CurrentSlotCount;
		if (rq_shader.RequiresZSorting == false)	{ if (model_count == 0U) continue; }
		else										{ if (rq_shader.SortedInstances.empty()) continue; }

		// Set any engine properties required by this specific shader
		if (rq_shader.AlphaBlendRequired != m_D3D->GetAlphaBlendState())
			m_D3D->SetAlphaBlendState(rq_shader.AlphaBlendRequired);

		// If this is a shader that requires z-sorting, perform that sort and render by the sorted method.  We can 
		// then skip the remainder of the process for this shader and move on to the next one
		if (rq_shader.RequiresZSorting) { PerformZSortedRenderQueueProcessing(rq_shader); continue; }

		// Iterate through each model queued for rendering by this shader
		for (auto mi = 0U; mi < model_count; ++mi)
		{
			// Store a reference to the model buffer currently being rendered
			m_current_modelbuffer = m_renderqueue[i].ModelData[mi].ModelBufferInstance;
			
			// Get the number of instances to be rendered
			instancecount = m_renderqueue[i].ModelData[mi].CurrentInstanceCount;
			RJ_FRAME_PROFILER_OUTPUT(concat("Activating model \"")(m_current_modelbuffer->GetCode())("\" (")(&(m_current_modelbuffer))(") [")(instancecount)(" instances]\n").str().c_str())

			// Sort all instances before rendering
			// TODO: In future consider maintaining a separate sorted index vector, if more efficient than a one-time post-sort
			m_renderqueue[i].ModelData[mi].SortInstances();

			// Loop through the instances in batches, if the total count is larger than our limit
			for (inst = 0U; inst < instancecount; inst += Game::C_INSTANCED_RENDER_LIMIT)
			{
				// Record render time if profiling is enabled
				RJ_FRAME_PROFILER_EXECUTE(render_time = Timers::GetHRClockTime();)

				// Determine the number of instances to render; either the per-batch limit, or fewer if we do not have that many
				n = min(instancecount - inst, Game::C_INSTANCED_RENDER_LIMIT);
				
				// Update the instance buffer by mapping, updating and unmapping the memory
				memset(&mappedres, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
				r_devicecontext->Map(m_instancebuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedres);
				memcpy(mappedres.pData, &(m_renderqueue[i].ModelData[mi].InstanceData[inst]), sizeof(RM_Instance) * n);
				r_devicecontext->Unmap(m_instancebuffer, 0);

				// Update the model VB pointer and then set vertex buffer data
				m_instancedbuffers[0] = m_current_modelbuffer->VertexBuffer.GetCompiledBuffer();
				m_instancedstride[0] = m_current_modelbuffer->VertexBuffer.GetStride();
				r_devicecontext->IASetVertexBuffers(0, 2, m_instancedbuffers, m_instancedstride, m_instancedoffset);

				// Set the model index buffer to active in the input assembler
				r_devicecontext->IASetIndexBuffer(m_current_modelbuffer->IndexBuffer.GetCompiledBuffer(), DXGI_FORMAT_R16_UINT, 0);

				// Set the type of primitive that should be rendered from this vertex buffer, if it differs from the current topology
				if (rq_shader.PrimitiveTopology != m_current_topology) 
					r_devicecontext->IASetPrimitiveTopology(rq_shader.PrimitiveTopology);

				// Now process all instanced / indexed vertex data through this shader
				rq_shader.Shader->Render(	r_devicecontext, m_current_modelbuffer->GetIndexCount(),
											m_current_modelbuffer->GetIndexCount(), (UINT)n,
											r_view, r_projection, m_current_modelbuffer->GetTextureResource());

				// Increment the count of draw calls that have been processed
				++m_renderinfo.DrawCalls;

				// Record profiling information if enabled this frame
				RJ_FRAME_PROFILER_OUTPUT(concat("> Rendering batch ")(inst)(" to ")(n - inst)(" [Shader=")(i)(", Model=\"")
					(m_current_modelbuffer ? m_current_modelbuffer->GetCode() : "NULL")("\"] = ")(Timers::GetMillisecondDuration(render_time, Timers::GetHRClockTime()))("ms\n").str().c_str())
			}
			
			// Update the total count of instances that have been processed
			m_renderinfo.InstanceCount += instancecount;

			// Finally, clear the instance data for this shader/model now that we have fully processed it
			m_renderqueue.UnregisterModelBuffer(i, mi);
		}

		// Reset this render queue shader ready for the next frame
		m_renderqueue[i].CurrentSlotCount = 0U;
	}	

	// Reset the render queue itself ready for the next frame
	m_current_modelbuffer = NULL;

	// Return any render parameters to their default if required, to avoid any downstream impact
	if (m_D3D->GetAlphaBlendState() != D3DMain::AlphaBlendState::AlphaBlendDisabled) m_D3D->SetAlphaBlendModeDisabled();
}*/

// Performs an intermediate z-sorting of instances before populating and processing the render queue.  Used only for 
// shaders/techniques (e.g. alpha blending) that require instances to be z-sorted.  Takes the place of normal rendering
void CoreEngine::PerformZSortedRenderQueueProcessing(RM_InstancedShaderDetails & shader)
{
	/*unsigned int n;
	std::vector<RM_Instance> renderbuffer;
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
			renderbuffer.emplace_back(std::move(shader.SortedInstances[i].Item));
		}

		// If this is an instance of a different model, or is the dummy end-element, we want to render the buffer that has been accumulated so far
		if (i == -1 || shader.SortedInstances[i].ModelPtr != m_current_modelbuffer)
		{
			// We are at this point because (a) we are at the end of the vector, or (b) the model has changed for a valid reason
			// We therefore want to render the buffer now.  Make sure that the buffer actually contains items 
			n = (unsigned int)renderbuffer.size();
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
				r_devicecontext->IASetIndexBuffer(m_current_modelbuffer->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

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

				// Update the actual number of instances that have been processed
				m_renderinfo.InstanceCountZSorted += n;
			}
			
			// Clear the render buffer now that it has been rendered
			renderbuffer.clear();

			// We are either here because the model has changed, or we are at the -1 element.  If we are not at the -1 element
			// we now want to update the current model pointer, and add the current element to the render buffer as the first item
			if (i != -1)
			{
				m_current_modelbuffer = shader.SortedInstances[i].ModelPtr;
				renderbuffer.emplace_back(std::move(shader.SortedInstances[i].Item));
			}
		}
	}

	// Clear the sorted instance vector ready for the next frame
	shader.SortedInstances.clear();*/
}

// Generic iObject rendering method; used by subclasses wherever possible.  Returns a flag indicating whether
// anything was rendered
bool CoreEngine::RenderObject(iObject *object)
{
	// Quit immediately if the object does not exist, or if we have already rendered it
	if (!object || object->IsRendered()) return false;

	// Test whether this object is within the viewing frustrum; if not, no need to render it
	// TODO: Need to move this out of the method
	if (!m_frustrum->TestObjectVisibility(object)) return false;

	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("Rendered object \"")(object ? object->GetInstanceCode() : "<NULL>")("\"").str(),
	{
		// Mark the object as visible
		Game::MarkObjectAsVisible(object);

		// We are rendering this object, so call its pre-render update method
		object->PerformRenderUpdate();

		// Render either articulated or static model depending on object properties
		if (object->GetArticulatedModel())
		{
			RenderObjectWithArticulatedModel(object);
		}
		else
		{
			RenderObjectWithStaticModel(object);
		}

		// Mark the object as having been rendered
		object->MarkAsRendered();
	});

	return true;
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
		float alpha = object->Fade.GetFadeAlpha();	// TODO: need to use this value
		if (alpha < Game::C_EPSILON) return;			

		SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, object->GetModel(), 
			std::move(RM_Instance(object->GetWorldMatrix(), object->GetLastWorldMatrix(), object->GetInstanceFlags())), object->GetPosition());
	}
	else
	{
		if (object->Highlight.IsActive())
		{
			SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, object->GetModel(), NULL, 
				std::move(RM_Instance(object->GetWorldMatrix(), object->GetLastWorldMatrix(), 
					RM_Instance::CalculateSortKey(object->GetPosition()), object->Highlight.GetColour(), object->GetInstanceFlags())), 
				std::move(RM_InstanceMetadata(object->GetPosition(), object->GetCollisionSphereRadius()))
			);
		}
		else
		{
			SubmitForRendering(RenderQueueShader::RM_LightShader, object->GetModel(), NULL, 
				std::move(RM_Instance(object->GetWorldMatrix(), object->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(object->GetPosition()), object->GetInstanceFlags())),
				std::move(RM_InstanceMetadata(object->GetPosition(), object->GetCollisionSphereRadius()))
			);
		}
	}
}

// Render an object with an articulated model
void CoreEngine::RenderObjectWithArticulatedModel(iObject *object)
{
	// Parameter check
	if (!object) return;

    // Perform an update of the articulated model to ensure all components are correctly positioned
	ArticulatedModel *model = object->GetArticulatedModel();
	if (!model) return;
	model->Update(object->GetPosition(), object->GetOrientation(), object->GetWorldMatrix());
	
    // Cache data for efficiency
    int n = model->GetComponentCount();
	InstanceFlags::Type instanceflags = object->GetInstanceFlags();

	// Add this object to the render queue for the relevant shader
	if (object->Fade.AlphaIsActive())
	{
		// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
		float alpha = object->Fade.GetFadeAlpha();	// TODO: need to use this alpha value
		if (alpha < Game::C_EPSILON) return;

        // Submit each component for rendering in turn
		ArticulatedModelComponent **component = model->GetComponents();
		for (int i = 0; i < n; ++i, ++component)
		{
			const auto comp = (*component);
			SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, (*component)->Model.GetModel(), std::move(
				RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), instanceflags)), comp->GetPosition());
		}
	}
	else
	{
		// Use same parent metadata for all components, since we don't currently have access to e.g. per-component extent
		RM_InstanceMetadata metadata(object->GetPosition(), object->GetCollisionSphereRadius());

		if (object->Highlight.IsActive())
		{
			const XMFLOAT4 & highlight = object->Highlight.GetColour();

			// Submit each component for rendering in turn
			ArticulatedModelComponent **component = model->GetComponents();
			for (int i = 0; i < n; ++i, ++component)
			{
				const auto comp = (*component);
				SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, comp->Model.GetModel(), NULL, std::move(
					std::move(RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), highlight, instanceflags))),
					std::move(RM_InstanceMetadata(metadata))
				);
			}
		}
		else
		{
			// Submit each component for rendering in turn
			ArticulatedModelComponent **component = model->GetComponents();
			for (int i = 0; i < n; ++i, ++component)
			{
				const auto comp = (*component);
				SubmitForRendering(RenderQueueShader::RM_LightShader, comp->Model.GetModel(), NULL, std::move(
					std::move(RM_Instance(comp->GetWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), instanceflags))),
					std::move(RM_InstanceMetadata(metadata))
				);
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

	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("-> Processed complex ship \"")(ship->GetInstanceCode())("\"").str(), 
	{
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
				// We want to attempt to render this ship section (keep track of whether at least one section was rendered)
				shiprendered |= RenderComplexShipSection(ship, sec);
			}
		}

		// We only need to render the ship & its contents if at least one ship section was rendered
		if (shiprendered)
		{
			// Set one active lighting configuration for the entire ship (rather than by turret) for efficiency
			//LightingManager.SetActiveLightingConfigurationForObject(ship);

			// Render any turret objects on the exterior of the ship, if applicable
			if (ship->TurretController.IsActive()) RenderTurrets(ship->TurretController);

			// Pass control to the environment-rendering logic to render all visible objects within the environment, if applicable.
			// Criteria are that either (a)it was requested, (b) the ship is or contains a simulation hub, or (c) the flag is set 
			// that forces the interior to always be rendered.  In addition, we must have actually rendered some part of the ship.
			if (renderinterior || is_hub || ship->InteriorShouldAlwaysBeRendered())
			{
				// Render the environment.  If we are also able to determine a more restrictive global view frustum
				// as part of this rendering then apply it here
				const Frustum *new_frustum = NULL;
				RenderEnvironment(ship, &new_frustum);
				if (new_frustum)
				{
					// TODO: Account for the new frustum
				}
			}

			// Mark the ship to indicate that it was visible and rendered this frame
			Game::MarkObjectAsVisible(ship);
			ship->MarkAsRendered();

			// Increment the complex ship render count if any of its sections were rendered this frame
			++m_renderinfo.ComplexShipRenderCount;
		}
	});
}
	
// Render a complex ship section to the space environment, as part of the rendering of the complex ship itself
// Returns a flag indicating whether the section was rendered
bool CoreEngine::RenderComplexShipSection(ComplexShip *ship, ComplexShipSection *sec)
{
	bool rendered = false;
	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("-> Processed complex ship section \"")(sec->GetInstanceCode())("\"").str(),
	{
		// Render the exterior of the ship, unless we have the relevant render flag set
		if (!m_renderflags[CoreEngine::RenderFlag::DisableHullRendering])
		{
			// Simply pass control to the main object rendering method
			rendered = RenderObject(sec);

			// Increment the render count
			if (rendered) ++m_renderinfo.ComplexShipSectionRenderCount;
		}
	});

	return rendered;
}

// Method to render the interior of an object environment, including any tiles, 
RJ_PROFILED(void CoreEngine::RenderEnvironment, iSpaceObjectEnvironment *environment, const Frustum **pOutGlobalFrustum)
{
	// Environment rendering has the potential to define constraints on the global view frustum
	const Frustum *new_frustum = NULL;

	// Use different rendering methods depending on the type of environment
	if (environment->SupportsPortalBasedRendering())
	{
		Result render_result = RenderPortalEnvironment(environment, 
			INITIAL_PORTAL_RENDERING_VIEWER_POSITION, 
			INITIAL_PORTAL_RENDERING_FRUSTUM, 
			&new_frustum);

		if (render_result == ErrorCodes::PortalRenderingNotPossibleInEnvironment)
		{
			RenderNonPortalEnvironment(environment, &new_frustum);
		}
	}
	else
	{
		RenderNonPortalEnvironment(environment, &new_frustum);
	}

	// Return any new (more restrictive) global visibility frustum that could be calculated during
	// environment rendering, or NULL to signify that no frustum change should be made
	(*pOutGlobalFrustum) = new_frustum;
}

/* Method to render the interior of an object environment including any tiles, for an environment
   which supports portal rendering
      - environment			The environment to be rendered
	  - view_position		Position of the viewer in world space
	  - initial_frustum		The initial view frustum, generally the global engine ViewFrustum
	  - pOutGlobalFrustum	Output parameter.  Passes a newly-constructed frustum object back to the 
							caller if rendering of the environment resulted in a more restrictive 
							global visibility frustum
      - Returns				A result code indicating whether the environment could be rendered
							via environment portal rendering
*/
Result CoreEngine::RenderPortalEnvironment(iSpaceObjectEnvironment *environment, const FXMVECTOR view_position, Frustum *initial_frustum, const Frustum **pOutGlobalFrustum)
{
	const TerrainDefinition *terrain_def = NULL;

	// Parameter check
	if (!environment || !initial_frustum) return ErrorCodes::CannotRenderNullEnvironment;

	// Precalculate the position of the viewer relative to the environment zero-point, so we can perform 
	// certain activities entirely in environment-local space
	XMVECTOR env_local_viewer = XMVector3TransformCoord(view_position, environment->GetInverseZeroPointWorldMatrix());

	// View position must be located within a valid element, that is itself within a cell
	// TODO: define non-tile cell type for e.g. viewer outside the environment or in interstitial space, looking in
	INTVECTOR3 viewer_location = environment->GetElementContainingPositionUnbounded(env_local_viewer);
	ComplexShipElement *el = environment->GetElement(viewer_location);
	if (el == NULL) return ErrorCodes::PortalRenderingNotPossibleInEnvironment;
	ComplexShipTile *current_cell = el->GetTile();
	if (current_cell == NULL) return ErrorCodes::PortalRenderingNotPossibleInEnvironment;

	// We will start with the current global visibility frustum, and potentially construct a more restrictive 
	// version during rendering.  The global view frustum exists in vector[0], and all >0 are temporary
	// frustums created during portal rendering that will be deallocated at the end of the method
	Frustum *new_global_frustum = NULL;
	m_tmp_frustums.clear();
	m_tmp_frustums.push_back(initial_frustum);
	std::vector<Frustum*>::size_type current_frustum_index = 0U;

	// Start with the current cell and proceed (vectorised) recursively
	DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat("Starting portal rendering cycle for environment \"")(environment->GetInstanceCode())("\"\n").str());
	std::vector<PortalRenderingStep> cells;
	cells.push_back(std::move(PortalRenderingStep(current_cell, current_frustum_index, 0U)));

	while (!cells.empty())
	{
		// Get the next cell to be processed
		PortalRenderingStep step = std::move(cells.back());
		cells.pop_back();
		DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat(" New cycle: ")(step.DebugString())("\n").str());

		// Make sure we have not passed the recursion limit for any particular traversal
		if (step.TraversalCount >= Game::C_MAX_PORTAL_RENDERING_DEPTH) continue;

		// Make sure the target cell exists
		// TODO: in future, may need to support transition into e.g. interstitial space or outside the environment, where cell == NULL
		ComplexShipTile *cell = step.Cell;
		if (!cell) continue;
		
		// Render the tile itself
		DEBUG_PORTAL_TRAVERSAL_LOG(environment, "   Rendering tile model\n");
		RenderComplexShipTile(cell, environment);

		// Get all objects within this tile area, which are visible based upon the current view frustum
		m_tmp_envobjects.clear(); m_tmp_terrain.clear();
		environment->GetAllVisibleObjectsWithinDistance(environment->SpatialPartitioningTree, cell->GetRelativePosition(),
			cell->GetBoundingSphereRadius(), m_tmp_frustums[step.VisibilityFrustum], &m_tmp_envobjects, &m_tmp_terrain);

		// Render all visible objects in the cell
		DEBUG_PORTAL_TRAVERSAL_LOG(environment, (m_tmp_envobjects.empty() ? "   Found no visible environment objects to render in cell\n" : concat("   Rendering ")(m_tmp_envobjects.size())(" visibile environment objects in cell\n").str().c_str()));
		for (auto *obj : m_tmp_envobjects)
		{ 
			RenderEnvironmentObject(obj);
		}

		// Render all visible terrain in the cell
		size_t terrain_count_rendered; DEBUG_PORTAL_TRAVERSAL(environment, terrain_count_rendered = m_renderinfo.TerrainRenderCount);
		DEBUG_PORTAL_TRAVERSAL_LOG(environment, (m_tmp_terrain.empty() ? "   Found no visible terrain objects to render in cell\n" : concat("   Processing ")(m_tmp_terrain.size())(" visibile terrain objects in cell\n").str().c_str()));
		for (auto *terrain : m_tmp_terrain)
		{
			if (!terrain) continue;
			if (terrain->IsRendered()) continue;
			if (terrain->IsDestroyed()) continue; 
			
			terrain_def = terrain->GetDefinition();
			if (terrain_def && terrain_def->HasModel())
			{
				// We want to render this terrain object; compose the terrain world matrix with its parent environment world matrix to get the final transform
				// Submit directly to the rendering pipeline.  Terrain objects are (currently) just a static model
				auto inst = RM_Instance(XMMatrixMultiply(terrain->GetWorldMatrix(), environment->GetZeroPointWorldMatrix()),
					RM_Instance::CalculateSortKey(XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(env_local_viewer, terrain->GetPosition())))), 
					terrain_def->GetInstanceFlags());
				auto worldpos = XMVector3TransformCoord(terrain->GetPosition(), environment->GetZeroPointWorldMatrix());

				SubmitForRendering(RenderQueueShader::RM_LightShader, terrain_def->GetModel(), NULL, 
					std::move(inst), 
					std::move(RM_InstanceMetadata(worldpos, terrain->GetCollisionSphereRadius()))
				);

				// This terrain object has been rendered
				terrain->MarkAsRendered();
				++m_renderinfo.TerrainRenderCount;
			}
			else if (terrain->HasArticulatedModel())
			{
				// Use same parent metadata for each component, since we don't have e.g. per-component extents right now
				auto worldpos = XMVector3TransformCoord(terrain->GetPosition(), environment->GetZeroPointWorldMatrix());
				RM_InstanceMetadata metadata(worldpos, terrain->GetCollisionSphereRadius());

				// Render each component of the articulated model
				ArticulatedModel *model = terrain->GetArticulatedModel();
				model->Update(worldpos,
					XMQuaternionMultiply(terrain->GetOrientation(), environment->GetOrientation()),
					XMMatrixMultiply(terrain->GetWorldMatrix(), environment->GetZeroPointWorldMatrix()));

				int n = model->GetComponentCount();
				InstanceFlags::Type instanceflags = (terrain_def != NULL ? terrain_def->GetInstanceFlags() : InstanceFlags::DEFAULT_INSTANCE_FLAGS);
				ArticulatedModelComponent **component = model->GetComponents();
				for (int i = 0; i < n; ++i, ++component)
				{
					auto comp = (*component);
					SubmitForRendering(RenderQueueShader::RM_LightShader, comp->Model.GetModel(), NULL, 
						std::move(RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), instanceflags)), 
						std::move(RM_InstanceMetadata(metadata))
					);
				}

				// This terrain object has been rendered
				terrain->MarkAsRendered();
				++m_renderinfo.TerrainRenderCount;
			}
		}
		DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat("   Rendered ")(m_renderinfo.TerrainRenderCount - terrain_count_rendered)(" terrain objects in cell\n").str());

		// Cell transform matrix is transformation from local coordinates to UN-rotated tile centre point, since portal data
		// is already rotated to the rotation state of the parent tile for evaluation-time efficiency
		XMMATRIX cell_transform = XMMatrixMultiply(cell->GetRelativePositionMatrix(), environment->GetZeroPointWorldMatrix());

		// Now process any portals in the current cell
		DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat("   Cell contains ")(cell->GetPortalCount())(" portals\n").str());
		for (const auto & portal : cell->GetPortals())
		{
			// Perform a basic sphere visibility test to quickly discard portals that are out of view
			DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat("   Processing ")(portal.DebugString())("\n").str());
			const XMVECTOR portal_centre = XMVector3TransformCoord(portal.GetCentrePoint(), cell_transform);
			if (m_tmp_frustums[step.VisibilityFrustum]->CheckSphere(portal_centre, portal.GetBoundingSphereRadius()) == false) {
				DEBUG_PORTAL_RENDER(portal, cell_transform, false);
				continue;
			}

			// Make sure that the portal is facing the viewer (i.e. dot(portal_to_viewer, portal_normal) must be > 0)
			// Perform comparison in world coordinates since we have both vectors already transformed into this space
			if (XMVector3LessOrEqual(XMVector3Dot(XMVectorSubtract(view_position, portal_centre), portal.GetNormal()), NULL_VECTOR)) {
				DEBUG_PORTAL_RENDER(portal, cell_transform, false);
				continue;
			}

			// Make sure the portal has a valid destination
			int target_element = portal.GetTargetLocation();
			ComplexShipElement *target_el = environment->GetElement(target_element);
			if (!target_el) continue;	// TODO: need to handle NULL destination in future, for e.g. interstitial space or portals to the outside
			ComplexShipTile *target_cell = target_el->GetTile();
			if (!target_cell) continue;	// TODO: need to handle NULL destination in future, for e.g. interstitial space or portals to the outside

			// Make sure we aren't transitioning to the same cell (e.g. due to an error in portal placement)
			if (target_cell == cell) continue;

			// Construct a new frustum by clipping against the portal bounds 
			assert(!(current_frustum_index >= 256U));		// Debug assertion; make sure this isn't getting out of control
			if (current_frustum_index >= 256U) continue;	// Make sure this isn't getting out of control
			Frustum *new_frustum = CreateClippedFrustum(view_position, *(m_tmp_frustums[step.VisibilityFrustum]), portal, cell_transform);
			++current_frustum_index;
			m_tmp_frustums.push_back(std::move(new_frustum));	// New item is at index current_frustum_index
			assert(m_tmp_frustums.size() == (current_frustum_index+1));
			
			// Use the new frustum to generate further steps in the portal traversal
			cells.push_back(std::move(PortalRenderingStep(target_cell, current_frustum_index, step.TraversalCount + 1)));
			DEBUG_PORTAL_RENDER(portal, cell_transform, true);
			DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat("   Generating new traversal step for portal: ")(cells.back().DebugString())("\n").str());
		}
	}

	// Rendering is complete; deallocate all interim view frustums that were created.  This is all frustums
	// except for [0], which is the global view frustum
	for (int i = 1; i <= current_frustum_index; ++i) delete m_tmp_frustums[i];
	m_tmp_frustums.clear();
	
	// If we are debug-rendering the portal data, also render all portals in tiles which were not processed
	DEBUG_PORTAL_TRAVERSAL(environment, { for (auto & tile : environment->GetTiles()) {
		if (tile.value && !tile.value->IsRendered()) {
			XMMATRIX cell_world = XMMatrixMultiply(tile.value->GetWorldMatrix(), environment->GetZeroPointWorldMatrix());
			for (auto & portal : tile.value->GetPortals()) DEBUG_PORTAL_RENDER(portal, cell_world, false); 
		}
	}});


	DEBUG_PORTAL_TRAVERSAL_LOG(environment, concat("Portal rendering complete for environment \"")(environment->GetInstanceCode())("\"\n").str());
	return NULL; // TODO
}

// Create a new view frustum by clipping the current frustum against the bounds of a view portal
// Portal is defined in cell-local space, and frustum is required in world space, so also accepts a cell-to-world transform
Frustum * CoreEngine::CreateClippedFrustum(const FXMVECTOR view_position, const Frustum & current_frustum, const ViewPortal & portal, const FXMMATRIX world_transform)
{
	// By constraining view portals to four vertices we can guarantee clipped view frustums are always four-sided
	Frustum *frustum = new Frustum(4U, current_frustum.GetNearPlane(), current_frustum.GetFarPlane());

	// Transform all portal vertices to world space
	XMVECTOR BL = XMVector3TransformCoord(portal.Vertices[0], world_transform);
	XMVECTOR TL = XMVector3TransformCoord(portal.Vertices[1], world_transform);
	XMVECTOR TR = XMVector3TransformCoord(portal.Vertices[2], world_transform);
	XMVECTOR BR = XMVector3TransformCoord(portal.Vertices[3], world_transform);

	// Clip each of the four frustum sides to the portal vertices. Each plane can be defined by three
	// points { view_pos, portal_vertex_0, portal_vertex_1 }.  Vertices must follow specific widing so
	// plane normals correctly face inwards
	frustum->SetPlane(Frustum::FIRST_SIDE + 0U, view_position, TR, TL);		// View-TR-TL
	frustum->SetPlane(Frustum::FIRST_SIDE + 1U, view_position, BR, TR);		// View-BR-TR
	frustum->SetPlane(Frustum::FIRST_SIDE + 2U, view_position, BL, BR);		// View-BL-BR
	frustum->SetPlane(Frustum::FIRST_SIDE + 3U, view_position, TL, BL);		// View-TL-BL

	// Return the new frustum.  It is the responsibility of the calling method to dispose of it afterwards
	return frustum;
}

// Debug-render an environment portal based on the given definition and world transform
void CoreEngine::DebugRenderPortal(const ViewPortal & portal, const FXMMATRIX world_matrix, bool is_active)
{
	// Transform all portal vertices to world space
	XMVECTOR BL = XMVector2TransformCoord(portal.Vertices[0], world_matrix);
	XMVECTOR TL = XMVector2TransformCoord(portal.Vertices[1], world_matrix);
	XMVECTOR TR = XMVector2TransformCoord(portal.Vertices[2], world_matrix);
	XMVECTOR BR = XMVector2TransformCoord(portal.Vertices[3], world_matrix);
	OverlayRenderer::RenderColour colour = (is_active ? OverlayRenderer::RenderColour::RC_Green : OverlayRenderer::RenderColour::RC_Red);

	// Render bounds of the portal
	Game::Engine->GetOverlayRenderer()->RenderLine(BL, TL, colour, 0.25f, -1.0f);
	Game::Engine->GetOverlayRenderer()->RenderLine(TL, TR, colour, 0.25f, -1.0f);
	Game::Engine->GetOverlayRenderer()->RenderLine(TR, BR, colour, 0.25f, -1.0f);
	Game::Engine->GetOverlayRenderer()->RenderLine(BR, BL, colour, 0.25f, -1.0f);

	// Also render the portal normal vector, slightly offset from centre so we can render opposing portals separately
	static const float RENDER_NORMAL_LENGTH = 3.0f;
	static const float RENDER_NORMAL_BREADTH = 0.25f;
	static const XMVECTOR RENDER_NORMAL_OFFSET = XMVectorSetX(NULL_VECTOR, RENDER_NORMAL_BREADTH * 0.5f);
	XMVECTOR centre = XMVector3TransformCoord(XMVectorAdd(portal.GetCentrePoint(), RENDER_NORMAL_OFFSET), world_matrix);
	XMVECTOR normal_pt = XMVectorMultiplyAdd(portal.GetNormal(), XMVectorReplicate(RENDER_NORMAL_LENGTH), centre);	// = (centre + (scalar * unit_normal))
	Game::Engine->GetOverlayRenderer()->RenderLine(centre, normal_pt, colour, RENDER_NORMAL_BREADTH, -1.0f);
}

// Set the debug level for portal rendering, if it has been enabled for a specific environment
void CoreEngine::SetDebugPortalRenderingConfiguration(bool debug_render, bool debug_log)
{
	m_debug_portal_debugrender = debug_render;
	m_debug_portal_debuglog = debug_log;
}

// Override the initial portal rendering frustum; relevant in debug builds only.  Override frustum will be deallocated by the
// engine at the end of the frame
void CoreEngine::DebugOverrideInitialPortalRenderingViewer(const iObject *viewer)
{
	if (!viewer)
	{
		m_debug_portal_render_initial_frustum = NULL;
		m_debug_portal_render_viewer_position = NULL_VECTOR;
		return;
	}

	// Override initial portal rendering frustum with one calculated for the given viewer
	XMMATRIX view, invview;
	GetCamera()->CalculateViewMatrixFromPositionData(viewer->GetPosition(), viewer->GetOrientation(), view, invview);
	
	assert(m_debug_portal_render_initial_frustum == NULL);
	m_debug_portal_render_initial_frustum = new Frustum(4U);
	m_debug_portal_render_initial_frustum->InitialiseAsViewFrustum(	m_renderdevice->GetProjectionMatrix(), Game::FarClipPlane,
																	m_renderdevice->GetFOV(), m_renderdevice->GetAspectRatio() );

	m_debug_portal_render_initial_frustum->ConstructViewFrustrum(view, invview);
	m_debug_portal_render_viewer_position = viewer->GetPosition();
}

/* Method to render the interior of an object environment including any tiles, for an environment
   which does not support portal rendering or where the viewer state does not permit it
      - environment:		The environment to be rendered
      - pOutGlobalFrustum:	Output parameter.  Passes a newly-constructed frustum object back to the
							caller if rendering of the environment resulted in a more restrictive
							global visibility frustum
*/
void CoreEngine::RenderNonPortalEnvironment(iSpaceObjectEnvironment *environment, const Frustum **pOutGlobalFrustum)
{
	// Parameter check
	if (!environment) return;

	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("-> Rendered environment \"")(environment->GetInstanceCode())("\" (without portal rendering support)").str(),
	{
		// Precalculate the environment (0,0,0) point in world space, from which we can calculate element/tile/object positions.  
		m_cache_zeropoint = XMVector3TransformCoord(NULL_VECTOR, environment->GetZeroPointWorldMatrix());

		// Also precalculate the position of the viewer relative to the environment zero-point, so we can perform 
		// certain activities entirely in environment-local space
		XMVECTOR env_local_viewer = XMVector3TransformCoord(Game::Engine->GetCamera()->GetPosition(), environment->GetInverseZeroPointWorldMatrix());

		// Also calculate a set of effective 'basis' vectors, that indicate the change in world space 
		// required to move in +x, +y and +z directions
		m_cache_el_inc[0].value = XMVector3TransformCoord(m_cache_el_inc_base[0].value, environment->GetOrientationMatrix());
		m_cache_el_inc[1].value = XMVector3TransformCoord(m_cache_el_inc_base[1].value, environment->GetOrientationMatrix());
		m_cache_el_inc[2].value = XMVector3TransformCoord(m_cache_el_inc_base[2].value, environment->GetOrientationMatrix());

		// Render all visible tiles
		RJ_FRAME_PROFILER_EXPR(int tiles_rendered = 0;)
		RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("Processed ")(environment->GetTileCount())(" tiles in environment").str(),
		{ 
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
				RJ_FRAME_PROFILER_EXECUTE(++tiles_rendered;)
				RenderComplexShipTile(tile, environment);
			}
		});
		RJ_FRAME_PROFILER_OUTPUT(concat(tiles_rendered)(" of ")(environment->GetTileCount())(" tiles rendered\n").str());

		// We perform a non-recursive vector traversal of the environment tree, and render the contents of any leaf nodes 
		// that are visible to the user
		RJ_FRAME_PROFILER_EXPR(int sectors_rendered = 0;)
		RJ_FRAME_PROFILER_PROFILE_BLOCK("Rendered environment sectors",
		{
			EnvironmentTree *node;
			m_tmp_envnodes.clear();
			m_tmp_envnodes.push_back(environment->SpatialPartitioningTree);
			while (!m_tmp_envnodes.empty())
			{
				// Get the next node to be processed
				node = m_tmp_envnodes.back();
				m_tmp_envnodes.pop_back();
				if (!node) continue;

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
					RJ_FRAME_PROFILER_EXECUTE(++sectors_rendered;)
					RenderObjectEnvironmentNodeContents(environment, node, env_local_viewer);
				}
			}
		});
		RJ_FRAME_PROFILER_OUTPUT(concat(sectors_rendered)(" environment sectors rendered\n").str());
	});
}

// Renders the entire contents of an environment tree node.  Internal method; no parameter checking
void CoreEngine::RenderObjectEnvironmentNodeContents(iSpaceObjectEnvironment *environment, EnvironmentTree *node, const FXMVECTOR environment_relative_viewer_position)
{
	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("Rendered environment sector containing ")(node->GetNodeObjects().size())(" objects, ")(node->GetTerrainCount())(" terrain objects (")
		(node->GetElementMin().ToString())(" to ")(node->GetElementMax().ToString())(")").str(), 
	{
		const TerrainDefinition *terrain_def = NULL;

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
		Terrain *terrain;
		std::vector<Terrain*>::const_iterator t_it_end = node->GetNodeTerrain().end();
		for (std::vector<Terrain*>::const_iterator t_it = node->GetNodeTerrain().begin(); t_it != t_it_end; ++t_it)
		{
			// Get a reference to the object and make sure it is valid, has a model etc.
			terrain = (*t_it);
			if (!terrain) continue;
			if (terrain->IsRendered()) continue;
			if (terrain->IsDestroyed()) continue;

			terrain_def = terrain->GetDefinition();
			if (terrain_def && terrain_def->HasModel())
			{
				// We want to render this terrain object; compose the terrain world matrix with its parent environment world matrix to get the final transform
				// Submit directly to the rendering pipeline.  Terrain objects are (currently) just a static model
				SubmitForRendering(RenderQueueShader::RM_LightShader, terrain_def->GetModel(), NULL, 
					std::move(RM_Instance(XMMatrixMultiply(terrain->GetWorldMatrix(), environment->GetZeroPointWorldMatrix()),
						RM_Instance::CalculateSortKey(XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(environment_relative_viewer_position, terrain->GetPosition())))), 
						terrain_def->GetInstanceFlags())), 
					std::move(RM_InstanceMetadata(terrain->GetPosition(), terrain->GetCollisionSphereRadius()))
				);

				// This terrain object has been rendered
				terrain->MarkAsRendered();
				++m_renderinfo.TerrainRenderCount;
			}
			else if (terrain->HasArticulatedModel())
			{
				// Use same parent metadata for all components, since we don't have e.g. per-component extents right now
				// TODO: Can take from [3] component of world matrix, as per tile rendering logic?
				RM_InstanceMetadata metadata(terrain->GetPosition(), terrain->GetCollisionSphereRadius());

				// Render all components of the articulated model
				ArticulatedModel *model = terrain->GetArticulatedModel();
				model->Update(XMVector3TransformCoord(terrain->GetPosition(), environment->GetZeroPointWorldMatrix()),
					XMQuaternionMultiply(terrain->GetOrientation(), environment->GetOrientation()),
					XMMatrixMultiply(terrain->GetWorldMatrix(), environment->GetZeroPointWorldMatrix()));

				int n = model->GetComponentCount();
				InstanceFlags::Type instanceflags = terrain_def->GetInstanceFlags();
				ArticulatedModelComponent **component = model->GetComponents();
				for (int i = 0; i < n; ++i, ++component)
				{
					auto comp = (*component);
					SubmitForRendering(RenderQueueShader::RM_LightShader, comp->Model.GetModel(), NULL, 
						std::move(RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), instanceflags)),
						std::move(RM_InstanceMetadata(metadata))
					);
				}

				// This terrain object has been rendered
				terrain->MarkAsRendered();
				++m_renderinfo.TerrainRenderCount;
			}

			
		}
	});
}

// Renders an object within a particular environment
void CoreEngine::RenderEnvironmentObject(iEnvironmentObject *object)
{
	// Parameter check
	if (!object) return;

	// Pass to different methods depending on the type of object
	switch (object->GetObjectType())
	{
		case iObject::ActorObject:
			QueueActorRendering((Actor*)object);				break;
	}
}

// Render a complex ship tile to the space environment, relative to its parent ship object
void CoreEngine::RenderComplexShipTile(ComplexShipTile *tile, iSpaceObjectEnvironment *environment)
{
	// Parameter check
	if (!tile) return;

	// Do not render anything if the tile has already been rendered this frame
	if (tile->IsRendered()) return;

	// Do not render anything if the tile has been destroyed (TODO: in future, render "destroyed" representation
	// of the tile and its contents instead
	if (tile->IsDestroyed()) return;

	// Calculate the absolute world matrix for this tile as (WM = Child * Parent)
	XMMATRIX world = XMMatrixMultiply(tile->GetWorldMatrix(), environment->GetZeroPointWorldMatrix());

	// From this point onwards we consider the tile to have been rendered
	tile->MarkAsRendered();

	// We are rendering this object, so call its pre-render update method
	tile->PerformRenderUpdate();

	// Handle differently depending on whether this is a single or compound tile
	if (!tile->HasCompoundModel())
	{
		// Add the single tile model to the render queue, for either normal or z-sorted processing
		if (tile->GetModel().GetModel()) 
		{
			if (tile->Fade.AlphaIsActive())
			{
				// Reject (alpha-clip) the object if its alpha value is effectively zero.  Otherwise alpha is passed as param.x
				float alpha = tile->Fade.GetFadeAlpha();	// TODO: need to use this alpha value
				if (alpha < Game::C_EPSILON) return;

				SubmitForZSortedRendering(	RenderQueueShader::RM_LightFadeShader, tile->GetModel().GetModel(), std::move(
					RM_Instance(world, RM_Instance::CalculateSortKey(world.r[3]), tile->GetInstanceFlags())), world.r[3]);			// Position can be taken from trans. components of world matrix (_41 to _43)
			}
			else
			{
				if (tile->Highlight.IsActive())
				{
					const XMFLOAT4 & highlight = tile->Highlight.GetColour();
					SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, tile->GetModel().GetModel(), NULL, 
						std::move(RM_Instance(world, RM_Instance::CalculateSortKey(world.r[3]), highlight,		// Position can be taken from trans. components of world matrix (_41 to _43)
							tile->GetInstanceFlags())),	
						std::move(RM_InstanceMetadata(world.r[3], tile->GetBoundingSphereRadius()))
					);			
				}
				else
				{
					SubmitForRendering(RenderQueueShader::RM_LightShader, tile->GetModel().GetModel(), NULL, 
						std::move(RM_Instance(world, RM_Instance::CalculateSortKey(world.r[3]), 				// Position can be taken from trans. components of world matrix (_41 to _43)
							tile->GetInstanceFlags())),				
						std::move(RM_InstanceMetadata(world.r[3], tile->GetBoundingSphereRadius()))
					);						
				}
			}
		}
	}
	else
	{	
		XMMATRIX modelwm; 

		// Determine whether any effects are active on the tile; these will need to be propogated across to constituent parts here
		bool fade = false; float alpha;
		if (tile->Fade.AlphaIsActive()) { fade = true; alpha = tile->Fade.GetFadeAlpha(); }

		// Iterate through all models to be rendered
		InstanceFlags::Type instanceflags = tile->GetInstanceFlags();
		for (auto & item : tile->GetCompoundModelSet().GetModels())
		{	
			// Calculate the full world transformation for this item
			modelwm = XMMatrixMultiply(
				item.LocalWorld,			// (Scale * LocalRotationWithinEnv * LocalTranslationWithinEnv * ...
				world						// ... WorldMatrixOfEnvironment)
			);
			
			// Submit the tile model for rendering using this adjusted world matrix
			// Take pos from the trans components of the world matrix (_41 to _43)
			if (fade)
			{
				SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, item.Model.GetModel(), std::move(
					RM_Instance(modelwm, RM_Instance::CalculateSortKey(modelwm.r[3]), instanceflags)), modelwm.r[3]);
			}
			else
			{
				SubmitForRendering(RenderQueueShader::RM_LightShader, item.Model.GetModel(), NULL, 
					std::move(RM_Instance(modelwm, RM_Instance::CalculateSortKey(modelwm.r[3]), instanceflags)), 
					std::move(RM_InstanceMetadata(modelwm.r[3], tile->GetBoundingSphereRadius()))
				);
			}
		}
	}

	// Increment the render count
	++m_renderinfo.ComplexShipTileRenderCount;
}

// Render a simple ship to the space environment
RJ_PROFILED(void CoreEngine::RenderSimpleShip, SimpleShip *s)
{
	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("-> Processed simple ship \"")(s ? s->GetInstanceCode() : "<NULL>")("\"").str(), 
	{
		// Simply pass control to the main object rendering method
		if (RenderObject(s) == true)
		{
			// Render any turret objects on the exterior of the ship, if applicable
			if (s->TurretController.IsActive()) RenderTurrets(s->TurretController);

			// Increment the render count
			++m_renderinfo.ShipRenderCount;
		}
	});
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
		
		// TODO: need to use this alpha value

		RJ_FRAME_PROFILER_PROFILE_BLOCK("Rendered turret objects (with alpha)", 
		{
			// Iterate through each turret in turn
			TurretController::TurretCollection::iterator it_end = controller.Turrets().end();
			for (TurretController::TurretCollection::iterator it = controller.Turrets().begin(); it != it_end; ++it)
			{
				// Get a reference to the model for this turret
				turret = (*it);
				model = turret->GetArticulatedModel(); if (!model) continue;
				n = model->GetComponentCount();
				InstanceFlags::Type instanceflags = turret->GetInstanceFlags();

				// Derive the turret world matrix since it is required for rendering (and we don't otherwise need it)
				turret->DetermineWorldMatrix();

				// Update the position of all model components before rendering
				model->Update(turret->GetPosition(), turret->GetOrientation(), turret->GetWorldMatrix());

				// Submit each component for rendering in turn
				component = model->GetComponents();
				for (int i = 0; i < n; ++i, ++component)
				{
					auto comp = (*component);
					SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, comp->Model.GetModel(), std::move(
						RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), instanceflags)), comp->GetPosition());
				}
			}
		});
	}
	else
	{
		//RJ_FRAME_PROFILER_PROFILE_BLOCK("Rendered turret objects",
		{
			if (parent->Highlight.IsActive())
			{
				const XMFLOAT4 & highlight = parent->Highlight.GetColour();

				// Iterate through each turret in turn
				TurretController::TurretCollection::iterator it_end = controller.Turrets().end();
				for (TurretController::TurretCollection::iterator it = controller.Turrets().begin(); it != it_end; ++it)
				{
					// Get a reference to the model for this turret
					turret = (*it);
					model = turret->GetArticulatedModel(); if (!model) continue;
					n = model->GetComponentCount();
					InstanceFlags::Type instanceflags = turret->GetInstanceFlags();

					// Derive the turret world matrix since it is required for rendering (and we don't otherwise need it)
					turret->DetermineWorldMatrix();

					// Update the position of all model components before rendering
					model->Update(turret->GetPosition(), turret->GetOrientation(), turret->GetWorldMatrix());

					// TODO: use same parent metadata for all components since we don't yet have required metadata at the component level
					auto extent = model->GetExtent();
					RM_InstanceMetadata metadata(turret->GetPosition(), max(max(extent.x, extent.y), extent.z));

					// Submit each component for rendering in turn
					component = model->GetComponents();
					for (int i = 0; i < n; ++i, ++component)
					{
						auto comp = (*component);
						SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, comp->Model.GetModel(), NULL, 
							std::move(RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), 
								highlight, instanceflags)), 
							std::move(RM_InstanceMetadata(metadata))
						);
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
					InstanceFlags::Type instanceflags = turret->GetInstanceFlags();

					// Derive the turret world matrix since it is required for rendering (and we don't otherwise need it)
					turret->DetermineWorldMatrix();

					// Update the position of all model components before rendering
					model->Update(turret->GetPosition(), turret->GetOrientation(), turret->GetWorldMatrix());

					// TODO: use same parent metadata for all components since we don't yet have required metadata at the component level
					auto extent = model->GetExtent();
					RM_InstanceMetadata metadata(turret->GetPosition(), max(max(extent.x, extent.y), extent.z));

					// Submit each component for rendering in turn
					component = model->GetComponents();
					for (int i = 0; i < n; ++i, ++component)
					{
						auto comp = (*component);
						SubmitForRendering(RenderQueueShader::RM_LightShader, comp->Model.GetModel(), NULL, 
							std::move(RM_Instance(comp->GetWorldMatrix(), comp->GetLastWorldMatrix(), RM_Instance::CalculateSortKey(comp->GetPosition()), 
								instanceflags)), 
							std::move(RM_InstanceMetadata(metadata))
						);
					}
				}
			}
		}//);
	}
}


// Renders all elements of a projectile set which are currently visible
void CoreEngine::RenderProjectileSet(BasicProjectileSet & projectiles)
{
	// Make sure the collection is active (which guarantees that it contains >0 projectiles)
	if (!projectiles.Active) return;

	RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("Rendered projectile set [")(projectiles.GetActiveProjectileCount())("]").str(), 
	{
		// Iterate through each element of the projectile set
		for (std::vector<BasicProjectile>::size_type i = 0; i <= projectiles.LiveIndex; ++i)
		{
			// Test visibility; we will only render projectiles within the viewing frustum
			BasicProjectile & proj = projectiles.Items[i];
			if (m_frustrum->CheckSphere(proj.Position, proj.Speed) == false) continue;

			// Submit directly for rendering using an instance generated from this projectile and the cached model/texture data in the projectile definition
			SubmitForZSortedRendering(RenderQueueShader::RM_VolLineShader, proj.Definition->Buffer, std::move(proj.GenerateRenderInstance()), proj.Position);
		}
	});
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
	m.Transform = XMFLOAT4X4(p1.x, p1.y, p1.z, 1.0f, p2.x, p2.y, p2.z, 1.0f, line.Colour.x, line.Colour.y, line.Colour.z, line.Colour.w, 0.0f, 0.0f, 0.0f, 0.0f);

	// Provide additional parameters as required.  x = line radius
	//m.Params = line.Params;		// TODO: need to re-enable this
	
	// Submit to the render queue
	SubmitForZSortedRendering(RenderQueueShader::RM_VolLineShader, VolLineShader::LineModel(line.RenderMaterial), std::move(m), line.P1);
}

// Constant adjustment such that screen rendering has (0,0) at top-left corner.  Adjusts (0,0) to (-ScreenWidth/2, +ScreenHeight/2)
// and (ScreenWidth, ScreenHeight) to (+ScreenWidth/2, -ScreenHeight/2)
void CoreEngine::RecalculateScreenSpaceAdjustment(void)
{
	// Float2 representation
	m_screen_space_adjustment_f = XMFLOAT2(Game::ScreenWidth * -0.5f, Game::ScreenHeight - (Game::ScreenHeight * 0.5f));

	// Ensure the z & w are set to zero so that we can ignore these components at runtime
	XMFLOAT4 adj = XMFLOAT4(m_screen_space_adjustment_f.x, m_screen_space_adjustment_f.y, 0.0f, 0.0f);
	m_screen_space_adjustment = XMLoadFloat2(&m_screen_space_adjustment_f);
}

RJ_PROFILED(void CoreEngine::RenderImmediateRegion, void)
{
	// Prepare all region particles, calculate vertex data and promote all to the buffer ready for shader rendering
	/*D::Regions::Immediate->Render(r_devicecontext, r_view);

	// Enable alpha blending before rendering any particles
	m_D3D->SetAlphaBlendModeEnabled();
	m_D3D->DisableZBufferWriting();

	// Render dust particles using the particle shader.  World matrix is set to the ID since all computations are in local/view space
	m_particleshader->Render( r_devicecontext, D::Regions::Immediate->GetActiveParticleCount() * 6, 
							  ID_MATRIX, r_view, r_projection, D::Regions::Immediate->GetParticleTextureResource() );

	// Disable alpha blending after rendering the particles
	// TODO: Move to somewhere more global once we have further rendering that requires alpha effects.  Avoid multiple on/off switches
	m_D3D->SetAlphaBlendModeDisabled();	
	m_D3D->EnableZBuffer();*/
}

RJ_PROFILED(void CoreEngine::RenderSystemRegion, void)
{
	// Use ID for the world matrix, except use the last (translation) row of the inverse view matrix for r[3] (_41 to _43)
	/*XMMATRIX world = ID_MATRIX;
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
		m_texcubeshader->Render( r_devicecontext, D::Regions::System->GetBackdropSkybox()->Data.IndexBuffer.GetIndexCount(), 
								 world, r_view, r_projection,
								 D::Regions::System->GetBackdropTextureResource() );

		// Re-enable culling and the z-buffer after rendering the skybox
		m_D3D->EnableRasteriserCulling();
		m_D3D->EnableZBuffer();
	}*/
}

RJ_PROFILED(void CoreEngine::RenderUserInterface, void)
{
	// Call the UI controller rendering function; this will invoke the Render() method of all UI controllers, 
	// allowing them to perform any render updates for the frame, as well as handling managed control state updates
	D::UI->Render();

	// Render all active 2D components to the engine render queue
	m_render2d->Render();

	// Render text strings to the interface
	// TODO: DISABLED
	//m_textmanager->Render(ID_MATRIX, r_orthographic);
}

RJ_PROFILED(void CoreEngine::RenderEffects, void)
{
	// Enable alpha blending before rendering any effects
	/*m_D3D->SetAlphaBlendModeEnabled();

	// Update the effect manager frame timer to ensure effects are rendered consistently regardless of FPS
	m_effectmanager->BeginEffectRendering();
	
	// Disable alpha blending after all effects are rendered
	m_D3D->SetAlphaBlendModeDisabled();*/
}

RJ_PROFILED(void CoreEngine::RenderParticleEmitters, void)
{
	// Pass control to the particle engine to perform all required rendering
	/*
	m_particleengine->Render(r_view, r_projection, m_D3D, m_camera);
	*/
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
	/*if (m_queuedactors.empty()) return;

	// Disable rasteriser culling to prevent loss of intermediate faces in skinned models; we only have to do this once before rendering all actors
	m_D3D->DisableRasteriserCulling();

	// Get parameters that are used for every actor being rendered.  This is done once before rendering all actors, to improve efficiency
	XMFLOAT3 campos; XMStoreFloat3(&campos, m_camera->GetPosition());
	
	// Iterate through the vector of queued actors
	Actor *actor;
	std::vector<Actor*>::iterator it_end = m_queuedactors.end();
	for (std::vector<Actor*>::iterator it = m_queuedactors.begin(); it != it_end; ++it)
	{
		actor = (*it);
		RJ_FRAME_PROFILER_PROFILE_BLOCK(concat("Rendered skinned actor \"")(actor->GetInstanceCode())("\"").str(),
		{
			// Calculate new transforms for the bones & mesh of this actor
			actor->UpdateForRendering(Game::TimeFactor);

			// Determine a lighting configuration for this actor
			LightingManager.SetActiveLightingConfigurationForObject(actor);

			// Render using the skinned model shader
			m_skinnedshader->Render(r_devicecontext, (*it)->GetModel(), campos, r_view_f, r_projection_f);
		});
	}

	// Enable rasteriser culling again once all actors have been rendered
	m_D3D->EnableRasteriserCulling();

	// Increase the count of actors we processed; we can count every item that was in the queue, since they 
	// were validated before being placed in the queue so should all render.  Also updated the skinned model
	// instance count since, for now, this is identical to the actor count
	size_t count = m_queuedactors.size();
	m_renderinfo.ActorRenderCount += count;
	m_renderinfo.ActorRenderCount += count;

	// Reset the queue now that it has been processed, so we are ready for adding new items in the next frame
	m_queuedactors.clear();*/
}

// Resets the queue to a state before any rendering has taken place.  Deallocates reserved memory.  
// Not reuired at shutdown since all resources are automatically deallocated; only for debug purposes
void CoreEngine::DeallocateRenderingQueue(void)
{
	// Iterate through each shader in the render queue
	size_t count = m_renderqueue.size();
	for (size_t shader = 0U; shader < count; ++shader)
	{
		// Iterate through each model currently registered with the shader
		auto model_count = m_renderqueue[shader].CurrentSlotCount;
		for (auto model = 0U; model < model_count; ++model)
		{
			// Iterate through each material registered for this model
			for (auto & material : m_renderqueue[shader].ModelData[model].Data)
			{
				// Clear all instance data
				material.InstanceCollection.InstanceData.clear();
				material.InstanceCollection.InstanceData.shrink_to_fit();

			} /// per-material

			// Unregister any model buffer associated with this entry
			if (m_renderqueue[shader].ModelData[model].ModelBufferInstance != NULL)
			{
				m_renderqueue.UnregisterModelBuffer(shader, model);
			}
		
		} /// per-model

		m_renderqueue[shader].ModelData.clear();
		m_renderqueue[shader].ModelData.shrink_to_fit();

	} /// per-shader

	m_renderqueue.clear();
	m_renderqueue.shrink_to_fit();
}

// Pre-render debug processes; only active in debug builds
void CoreEngine::RunPreRenderDebugProcesses(void)
{

}

// Post-render debug processes; only active in debug builds
void CoreEngine::RunPostRenderDebugProcesses(void)
{
	// Clear per-frame debug state
	if (m_debug_portal_render_initial_frustum)
	{
		SafeDelete(m_debug_portal_render_initial_frustum);
		m_debug_portal_render_viewer_position = NULL_VECTOR;
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
	
	if (m_renderflags[CoreEngine::RenderFlag::RenderCachedFrustum]) DebugRenderFrustum(false);
	else if (m_renderflags[CoreEngine::RenderFlag::RenderFrustum])  DebugRenderFrustum(true);
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
		count = 1 + Game::Search<iObject>().GetAllObjectsWithinDistance(Game::CurrentPlayer->GetParentEnvironment(), 10000.0f, objects,
																			(Game::ObjectSearchOptions::OnlyCollidingObjects));

		// Also include the parent ship environmment (hence why we +1 to the count above)
		objects.push_back((iObject*)Game::CurrentPlayer->GetParentEnvironment());
	}
	else
	{
		// Player is in a spaceobject ship, so use the proximity test on their ship
		if (Game::CurrentPlayer->GetPlayerShip() == NULL) return;
		count = 1 + Game::Search<iObject>().GetAllObjectsWithinDistance(Game::CurrentPlayer->GetPlayerShip(), 10000.0f, objects, 
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
	AXMVECTOR_P v[8];

	// Get a reference to the environment object
	iSpaceObjectEnvironment *parent = (iSpaceObjectEnvironment*)Game::GetObjectByID(m_debug_renderenvboxes);
	if (!parent) return;

	// Get a reference to all objects within a radius of the camera
	std::vector<iEnvironmentObject*> objects; 
	std::vector<Terrain*> terrain;
	XMVECTOR cam_pos = XMVector3TransformCoord(GetCamera()->GetPosition(), parent->GetInverseZeroPointWorldMatrix());
	parent->GetAllObjectsWithinDistance(parent->SpatialPartitioningTree, cam_pos, Game::C_DEBUG_RENDER_ENVIRONMENT_COLLISION_BOX_RADIUS, &objects, &terrain);

	// Iterate through all active objects within this parent environment
	for (iEnvironmentObject *obj : objects)
	{
		if (!obj) continue;

		// Determine the location of all vertices for each bounding volume and render using the overlay renderer
		obj->CollisionOBB.DetermineVertices(v);
		Game::Engine->GetOverlayRenderer()->RenderCuboid(v, OverlayRenderer::RenderColour::RC_LightBlue, 0.1f);
	}

	// Iterate through all terrain objects within this parent environment
	for (Terrain *tobj : terrain)
	{
		if (!tobj || tobj->IsDestroyed()) continue;

		if (m_debug_terrain_render_mode == DebugTerrainRenderMode::Solid)
		{
			// Perform solid rendering
			Game::Engine->GetOverlayRenderer()->RenderCuboid(XMMatrixMultiply(tobj->GetWorldMatrix(), parent->GetZeroPointWorldMatrix()),
				OverlayRenderer::RenderColour::RC_Red, XMVectorScale(XMLoadFloat3(&(tobj->GetExtentF())), 2.0f));
		}
		else
		{
			// Determine the location of all vertices for each bounding volume and render wireframe using the overlay renderer
			tobj->DetermineCollisionBoxVertices(parent, v);
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
		m_overlayrenderer->RenderNode(pos, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

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
// TODO [textrender]: Redo this whole method with the new text rendering components
void CoreEngine::DebugRenderObjectIdentifiers(void)
{
	/*static const XMFLOAT2 DEBUG_ID_TEXT_OFFSET = XMFLOAT2(0.0f, 0.0f);
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
			SentenceType *s = m_textmanager->CreateSentence(Font::DEFAULT_FONT_ID, 256);
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
		int n = Game::Search<iObject>().GetAllObjectsWithinDistance(pos, node, m_debug_renderobjid_distance, spaceobjects, Game::ObjectSearchOptions::NoSearchOptions);

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
	}*/
}

void CoreEngine::DebugRenderFrustum(bool update_cache)
{
	if (update_cache)
	{
		DebugUpdateFrustumRenderCache();
	}

	/* Frustum points should be connected as follows: 
		0-1, 1-2, 2-3, 3-0, 
		4-5, 5-6, 6-7, 7-4,
		0-4, 1-5, 2-6, 3-7
	*/

	static const int edges[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, 
							  		  { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, 
									  { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

	for (int i = 0; i < 12; ++i)
	{
		Game::Engine->GetOverlayRenderer()->RenderLine(m_debug_frustum_render[edges[i][0]], m_debug_frustum_render[edges[i][1]],
			OverlayRenderer::RenderColour::RC_Green, 10.0f, -1.0f);
	}
}

void CoreEngine::DebugUpdateFrustumRenderCache(void)
{
	Game::Engine->GetViewFrustrum()->DetermineWorldSpaceCorners(m_debug_frustum_render);
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
	else if (command.InputCommand == "portal_render")
	{
		iObject *obj = command.ParameterAsObject(0);
		if (!obj || !obj->IsEnvironment()) {
			SetDebugPortalRenderingTarget(0U);
			SetDebugPortalRenderingConfiguration(false, false);
			command.SetSuccessOutput("Disabling portal debug rendering (usage = \"portal_render <obj-id> <debug-render> <debug-log>\"; obj-id == 0 to disable)");
			return true;
		}

		bool debug_render = command.ParameterAsBool(1);
		bool debug_log = command.ParameterAsBool(2);
		if (!(debug_render || debug_log)) {
			command.SetSuccessOutput("Disabling portal debug rendering (usage = \"portal_render <obj-id> <debug-render> <debug-log>\"; obj-id == 0 to disable)");
			return true;
		}

		SetDebugPortalRenderingTarget(obj->GetID());
		SetDebugPortalRenderingConfiguration(debug_render, debug_log);
		command.SetSuccessOutput(concat("Debug portal rendering enabled for \"")(obj->GetInstanceCode())("\" (")(obj->GetID())("); rendering ")
			(debug_render ? "enabled" : "disabled")(", logging ")(debug_log ? "enabled" : "disabled").str());
		return true;
	}
	else if (command.InputCommand == "render_frustum")
	{
		if (command.Parameter(0) == "current") {
			SetRenderFlag(CoreEngine::RenderFlag::RenderFrustum, true);
			command.SetSuccessOutput("Enabling debug render of current view frustum");
		}
		else if (command.Parameter(0) == "cached") {
			DebugUpdateFrustumRenderCache();
			SetRenderFlag(CoreEngine::RenderFlag::RenderCachedFrustum, true);
			command.SetSuccessOutput("Enabling debug render of view frustum, cached for the current frame");
		}
		else {
			SetRenderFlag(CoreEngine::RenderFlag::RenderFrustum, false);
			SetRenderFlag(CoreEngine::RenderFlag::RenderCachedFrustum, false);
			command.SetSuccessOutput("Disabling debug render of view frustum");
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

// Outputs the contents of the render queue to debug-out
void CoreEngine::DebugOutputRenderQueueContents(void)
{
	std::ostringstream ss;
	ss << "Render Queue Shaders\n" <<
		"{\n";

	for (int i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		ss << "\tShader " << i << 
			" [ZSort=" << m_renderqueueshaders[i].RequiresZSorting <<
			", Topology=" << m_renderqueueshaders[i].PrimitiveTopology << "]\n";
	}
	ss << "}\n";

	OutputDebugString(ss.str().c_str());
}

// Default destructor
CoreEngine::~CoreEngine(void)
{
}

