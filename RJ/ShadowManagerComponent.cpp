#include "ShadowManagerComponent.h"
#include "CoreEngine.h"
#include "RenderProcessDX11.h"
#include "RenderQueue.h"
#include "RM_ModelDataCollection.h"
#include "RM_MaterialInstanceMapping.h"
#include "RM_InstanceData.h"
#include "Light.h"
#include "CameraView.h"
#include "CameraProjection.h"
#include "IntersectionTests.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "ShaderDX11.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "PipelineStateDX11.h"
#include "GameConsoleCommand.h"

// TODO (SM): remove
//#include "OverlayRenderer.h"


// Initialise static data
const std::string ShadowManagerComponent::TX_SHADOWMAP_DEBUG_CAPTURE = "ShadowMap_Debug_Capture_TX";
const std::string ShadowManagerComponent::RP_SHADOWMAP = "ShadowMap_Pipeline";

const float ShadowManagerComponent::DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_DIST = 1.0f;
const float ShadowManagerComponent::DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_SIDE_SCALING = 1.0f;


// Default shadow map sizes per light type
const ShadowManagerComponent::SMSize ShadowManagerComponent::DEFAULT_SHADOW_MAP_SIZE[static_cast<int>(LightType::_LightTypeCount)] =
{ 
	SM2048,		// 0: Point
	SM2048,		// 1: Spotlight
	SM4096		// 2: Directional
};

// Constructor
ShadowManagerComponent::ShadowManagerComponent(void)
{
	// Only for default intialisation
}

// Constructor
ShadowManagerComponent::ShadowManagerComponent(DeferredRenderProcess *renderprocess)
	:
	m_renderprocess(renderprocess),

	m_vs_lightspace_shadowmap(NULL),
	m_pipeline_lightspace_shadowmap(NULL),

	m_cb_lightspace_shadowmap(NULL),
	m_cb_shadowmapped_light(NULL), 
	
	r_viewfrustum(NULL),
	r_frustum_world_min(NULL_VECTOR),
	r_frustum_world_max(NULL_VECTOR),
	m_lightspace_view_neardist(DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_DIST),
	r_lightspace_view_fardist(DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_DIST + 1.0f), 
	r_frustum_longest_diagonal_mag(0.0f), 
	m_lightspace_view_nearside_scaling(DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_SIDE_SCALING), 

	m_current_shadow_map_size(SMSize::SMUnknown),
	m_standard_shadow_map_size{ DEFAULT_SHADOW_MAP_SIZE[0], DEFAULT_SHADOW_MAP_SIZE[1], DEFAULT_SHADOW_MAP_SIZE[2] },

	m_instance_capacity(0U), 

	m_active_sm_light_position(NULL_VECTOR), 
	m_active_sm_light_orientation(ID_QUATERNION), 
	m_active_sm_viewproj(ID_MATRIX), 

	m_param_vs_shadowmap_smdata(ShaderDX11::INVALID_SHADER_PARAMETER), 

	m_debug_shadowmap_capture(NULL), 
	m_debug_shadowmap_capture_id(DEBUG_SHADOW_MAP_CAPTURE_DISABLED), 
	m_debug_shadowmap_capture_size(SMSize::SMUnknown)
{
	for (int i = 0; i < 8; ++i) r_frustum_world_corners[i] = NULL_VECTOR;

	// Initialisation
	InitialiseShadowMapConfigurations();
	InitialiseShaders();
	InitialiseStandardBuffers();
	InitialiseRenderQueueProcessing();

	PerformPostConfigInitialisation();
}

// Initialisation of all components which are config-dependent or should be refreshed on device parameters change (e.g. display size)
void ShadowManagerComponent::PerformPostConfigInitialisation(void)
{
	InitialiseTextureBuffers();
	InitialiseRenderTargets();
	InitialiseRenderPipelines();
}

void ShadowManagerComponent::InitialiseShadowMapConfigurations(void)
{
	Game::Log << LOG_INFO << "Initialising supported shadow map configurations\n";

	// Texel size for each configuration
	static const std::vector<std::tuple<SMSize, unsigned int>> sm_sizes = {
		{ SMSize::SMUnknown, 0U }, { SMSize::SM256, 256U }, { SMSize::SM512, 512U },
		{ SMSize::SM1024, 1024U }, { SMSize::SM2048, 2048U }, { SMSize::SM4096, 4096U } 
	};

	// Initialise each config in turn
	for (int i = 0U; i < static_cast<int>(SMSize::_SMCOUNT); ++i)
	{
		SMSize size = static_cast<SMSize>(i);
		unsigned int texel_size = std::get<1>( *std::find_if(sm_sizes.begin(), sm_sizes.end(), 
			[size](const auto & item) { return (std::get<0>(item) == size); } ));

		m_shadow_map_config[i].Size = size;
		m_shadow_map_config[i].TexelSize = texel_size;

		m_shadow_map_config[i].Viewport = Viewport(0.0f, 0.0f, static_cast<float>(texel_size), static_cast<float>(texel_size));
		m_shadow_map_config[i].ViewportCompiled = m_shadow_map_config[i].Viewport.CompileViewport();

		m_shadow_map_config[i].Texture = NULL;			// Assets will be initialised later
		m_shadow_map_config[i].RenderTarget = NULL;		// Assets will be initialised later
	}
}

void ShadowManagerComponent::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Intialising shadow mapping shaders\n";

	// Get a reference to all required shaders
	m_vs_lightspace_shadowmap = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::ShadowMappingVertexShader);
	if (m_vs_lightspace_shadowmap == NULL) Game::Log << LOG_ERROR << "Cannot load light-space shadow mapping shader resources [vs]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_shadowmap_smdata = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_vs_lightspace_shadowmap, LightSpaceShadowMapDataBufferName);
}

void ShadowManagerComponent::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Intialising shadow mapping standard buffers\n";

	m_cb_lightspace_shadowmap = Game::Engine->GetAssets().CreateConstantBuffer(LightSpaceShadowMapDataBufferName, m_cb_lightspace_shadowmap_data.RawPtr);
	m_cb_shadowmapped_light = Game::Engine->GetAssets().CreateConstantBuffer(ShadowMappedLightBufferName, m_cb_shadowmapped_light_data.RawPtr);
}


void ShadowManagerComponent::InitialiseTextureBuffers(void)
{
	Texture::TextureFormat & primary_colour_buffer_format = Game::Engine->GetRenderDevice()->PrimaryRenderTargetColourBufferFormat();
	Texture::TextureFormat & primary_dsv_buffer_format = Game::Engine->GetRenderDevice()->PrimaryRenderTargetDepthStencilBufferFormat();

	// Fixed texture resource details
	std::vector<std::tuple<const std::string, const std::string, TextureDX11**, Texture::TextureFormat&, IntegralVector2<unsigned int>>> textures = {
		// None right now
	};

	// Insert additional texture descriptions based on the supported shadow map configurations
	for (auto & config : m_shadow_map_config)
	{
		if (config.Size == SMSize::SMUnknown) continue;		// No asset required for 'unknown' state

		textures.push_back({ concat("light-space DSV (")(static_cast<int>(config.Size))("-")(config.TexelSize)(")").str(),
			ShadowmapTextureAssetName(config.Size),
			&(config.Texture),
			primary_dsv_buffer_format,
			UINTVECTOR2(config.TexelSize) 
		});
	}


	// Create each texture resource in turn
	for (const auto & tex : textures)
	{
		std::string desc = std::get<0>(tex);
		std::string name = std::get<1>(tex);
		TextureDX11 **ppTex = std::get<2>(tex);
		Texture::TextureFormat & format = std::get<3>(tex);
		IntegralVector2<unsigned int> size = std::get<4>(tex);

		// Release any resource that already exists, otherwise log on first creation
		if (Game::Engine->GetAssets().AssetExists<TextureDX11>(name))
		{
			Game::Engine->GetAssets().DeleteAsset<TextureDX11>(name);
		}
		else
		{
			Game::Log << LOG_INFO << "Initialising shadow mapping " << desc << " texture resource\n";
		}

		// Create the new resource
		*ppTex = Game::Engine->GetAssets().CreateTexture2D(name, size.x, size.y, 1U, format);
		if (!(*ppTex)) Game::Log << LOG_ERROR << "Failed to create shadow mapping " << desc << " texture resource\n";
	}
}

void ShadowManagerComponent::InitialiseRenderTargets(void)
{
	// Fixed render target data (desc, name, pRT, size, colour-buffer, dsv-buffer)
	std::vector<std::tuple<std::string, std::string, RenderTargetDX11**, IntegralVector2<int>, TextureDX11*, TextureDX11*>> rts = {
		// None right now
	};

	// Insert additional RT descriptions based on the set of supported shadow map configurations
	for (auto & config : m_shadow_map_config)
	{
		if (config.Size == SMSize::SMUnknown) continue;		// No asset required for 'unknown' state

		rts.push_back({
			concat("light-space render target (")(static_cast<int>(config.Size))("-")(config.TexelSize)(")").str(),
			ShadowManagerComponent::ShadowmapRenderTargetAssetName(config.Size),
			&(config.RenderTarget),
			INTVECTOR2(static_cast<int>(config.TexelSize)),
			NULL,
			config.Texture
		});
	}

	// Initialise each RT in turn
	for (const auto & rt : rts)
	{
		std::string desc = std::get<0>(rt);
		std::string name = std::get<1>(rt);
		RenderTargetDX11 **ppRT = std::get<2>(rt);
		IntegralVector2<int> size = std::get<3>(rt);
		TextureDX11 *txcolour = std::get<4>(rt);
		TextureDX11 *txdsv = std::get<5>(rt);

		// Release any resource that already exists, otherwise log on first creation
		if (Game::Engine->GetAssets().AssetExists<RenderTargetDX11>(name))
		{
			Game::Engine->GetAssets().DeleteAsset<RenderTargetDX11>(name);
		}
		else
		{
			Game::Log << LOG_INFO << "Initialising shadow mapping " << desc << " resource\n";
		}

		*ppRT = Game::Engine->GetAssets().CreateRenderTarget(name, size);
		if (*ppRT)
		{
			if (txcolour)	(*ppRT)->AttachTexture(RenderTarget::AttachmentPoint::Color0, txcolour);
			if (txdsv)		(*ppRT)->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, txdsv);
		}
		else
		{
			Game::Log << LOG_ERROR << "Failed to create shadow mapping " << desc << " resource\n";
		}
	}
}

void ShadowManagerComponent::InitialiseRenderPipelines(void)
{
	InitialiseLightSpaceShadowMappingPipeline();
}

void ShadowManagerComponent::InitialiseLightSpaceShadowMappingPipeline(void)
{
	// Recreate existing pipeline state if it already exists
	if (Game::Engine->GetAssets().AssetExists<PipelineStateDX11>(RP_SHADOWMAP))
	{
		Game::Engine->GetAssets().DeleteAsset<PipelineStateDX11>(RP_SHADOWMAP);
	}
	else
	{
		Game::Log << LOG_INFO << "Initialising light-space shadow mapping render pipeline\n";
	}

	m_pipeline_lightspace_shadowmap = Game::Engine->GetAssets().CreatePipelineState(RP_SHADOWMAP);
	if (!m_pipeline_lightspace_shadowmap)
	{
		Game::Log << LOG_ERROR << "Failed to initialise light-space shadow mapping render pipeline\n";
		return;
	}

	// General pipeline configuration
	m_pipeline_lightspace_shadowmap->SetShader(Shader::Type::VertexShader, m_vs_lightspace_shadowmap);
	m_pipeline_lightspace_shadowmap->SetRenderTarget(NULL);								// Default; will be overridden on first frame
	m_pipeline_lightspace_shadowmap->GetRasterizerState().SetViewport(Viewport());		// Default; will be overridden on first frame

	// Standard depth buffer operations; we want to render a closest-depth buffer from the light perspective
	DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Enable, DepthStencilState::CompareFunction::Less);
	m_pipeline_lightspace_shadowmap->GetDepthStencilState().SetDepthMode(depthMode);

	// Front-face culling so that backfaces are shadowed; reduces artifacts when compared to back-face culling
	m_pipeline_lightspace_shadowmap->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Front);
}

void ShadowManagerComponent::InitialiseRenderQueueProcessing(void)
{
	Game::Log << LOG_INFO << "Intialising shadow mapping render queue processing\n";

	// Preallocate space in the instance data collection
	m_instance_capacity = INITIAL_INSTANCE_DATA_CAPACITY; 
	m_instances.reserve(INITIAL_INSTANCE_DATA_CAPACITY);
	for (int i = 0; i < INITIAL_INSTANCE_DATA_CAPACITY; ++i)
	{
		m_instances.push_back(std::move(RM_Instance()));
	}	
}

// Returns a pointer to the active shadow map buffer resource
TextureDX11 * ShadowManagerComponent::GetActiveShadowMap(void)
{
	return m_shadow_map_config[static_cast<unsigned int>(m_current_shadow_map_size)].Texture;
}

// Return the texel size of the given SM variant
unsigned int ShadowManagerComponent::GetShadowMapSizeInTexels(SMSize size) const
{
	unsigned int index = static_cast<int>(size);
	if (index >= static_cast<unsigned int>(SMSize::_SMCOUNT))
	{
		return m_shadow_map_config[static_cast<unsigned int>(SMSize::SMUnknown)].TexelSize;
	}

	return m_shadow_map_config[index].TexelSize;
}

// Retrieve the SM variant corresponding to the given texel size
ShadowManagerComponent::SMSize ShadowManagerComponent::GetShadowMapSizeFromTexels(unsigned int tex_size) const
{
	for (const auto & entry : m_shadow_map_config)
	{
		if (entry.TexelSize == tex_size) return entry.Size;
	}

	return SMSize::SMUnknown;
}

// Returns the standard SM size for the given light type
ShadowManagerComponent::SMSize ShadowManagerComponent::GetStandardShadowMapSize(LightType light_type) const
{
	assert(static_cast<unsigned int>(light_type) < static_cast<unsigned int>(LightType::_LightTypeCount));

	return m_standard_shadow_map_size[static_cast<int>(light_type)];
}

// Returns the standard SM texel size for the given light type
unsigned int ShadowManagerComponent::GetStandardShadowMapTextureSize(LightType light_type) const
{
	return GetShadowMapSizeInTexels(GetStandardShadowMapSize(light_type));
}

// Set the standard shadow map size for the given light type
bool ShadowManagerComponent::SetStandardShadowMapSize(LightType light_type, SMSize size)
{
	if (static_cast<unsigned int>(light_type) >= static_cast<unsigned int>(LightType::_LightTypeCount))
	{
		Game::Log << LOG_WARN << "Cannot set standard shadow map size; invalid light type (" << static_cast<unsigned int>(light_type) << ")\n";
		return false;
	}

	if (static_cast<unsigned int>(size) >= static_cast<unsigned int>(SMSize::_SMCOUNT))
	{
		Game::Log << LOG_WARN << "Cannot set standard shadow map size; invalid shadow map size (" << static_cast<unsigned int>(size) << ")\n";
		return false;
	}

	if (size == SMSize::SMUnknown)
	{
		Game::Log << LOG_WARN << "Cannot set standard shadow map size to 'unknown' buffer size (" << static_cast<unsigned int>(size) << ")\n";
		return false;
	}

	m_standard_shadow_map_size[static_cast<int>(light_type)] = size;
	
	Game::Log << LOG_INFO << "Standard shadow map size (light-type: " << static_cast<unsigned int>(light_type) << ") set to "
		<< GetShadowMapSizeInTexels(size) << " (sm-size: " << static_cast<unsigned int>(size) << ")\n";
	return true;
}

// Set the standard shadow map size in texels for the given light type
bool ShadowManagerComponent::SetStandardShadowMapTextureSize(LightType light_type, unsigned int size)
{
	SMSize sm_size = GetShadowMapSizeFromTexels(size);
	if (sm_size == SMSize::SMUnknown)
	{
		Game::Log << LOG_WARN << "Cannot set shadow map size to " << size << "; invalid buffer size\n";
		return false;
	}

	return SetStandardShadowMapSize(light_type, sm_size);
}

// Reset to the standard shadow map size for the given light type
bool ShadowManagerComponent::ResetStandardShadowMapSize(LightType light_type)
{
	if (static_cast<unsigned int>(light_type) >= static_cast<unsigned int>(LightType::_LightTypeCount))
	{
		Game::Log << LOG_WARN << "Cannot reset shadow map size; invalid light type (" << static_cast<unsigned int>(light_type) << ")\n";
		return false;
	}

	return SetStandardShadowMapSize(light_type, DEFAULT_SHADOW_MAP_SIZE[static_cast<unsigned int>(light_type)]);
}

// Reset to the standard shadow map size for the given light type
void ShadowManagerComponent::ResetStandardShadowMapSizes(void)
{
	for (unsigned int light = 0U; light < static_cast<unsigned int>(LightType::_LightTypeCount); ++light)
	{
		ResetStandardShadowMapSize(static_cast<LightType>(light));
	}
}

// Perform per-frame initialisation before any shadow map rendering is performed
void ShadowManagerComponent::BeginFrame(void)
{
	// Initialise any per-frame fields
	m_active_shadow_map_count = 0U;

	// Get a reference to the view frustum and calculate any derived data
	CalculateViewFrustumData(Game::Engine->GetViewFrustrum());
}

// Perform per-frame calculations based on the current view frustum
void ShadowManagerComponent::CalculateViewFrustumData(const Frustum *frustum)
{
	assert(frustum);

	// Get a reference to the view frustum and its world-space coordinates
	r_viewfrustum = Game::Engine->GetViewFrustrum();
	r_viewfrustum->DetermineWorldSpaceCorners(r_frustum_world_corners);

	// Determine the minimum and maximum frustum extents in world-space
	XMVECTOR vsum = NULL_VECTOR;
	r_frustum_world_min = r_frustum_world_corners[0];
	r_frustum_world_max = r_frustum_world_corners[0];
	for (int i = 1; i < 8; ++i)
	{
		r_frustum_world_min = XMVectorMin(r_frustum_world_min, r_frustum_world_corners[i]);
		r_frustum_world_max = XMVectorMax(r_frustum_world_max, r_frustum_world_corners[i]);
		vsum = XMVectorAdd(vsum, r_frustum_world_corners[i]);
	}

	// Store the length of the longest view frustum diagonal, for use in sizing the light frustum
	r_frustum_longest_diagonal_mag = XMVector3Length(XMVectorSubtract(r_frustum_world_corners[6], r_frustum_world_corners[0]));
	r_frustum_centre_point = XMVectorDivide(vsum, XMVectorReplicate(8.0f));

	// Calculate camera, near and far plane distances
	XMVECTOR half_diag = XMVectorMultiply(r_frustum_longest_diagonal_mag.V(), HALF_VECTOR);
	XMVECTOR near_side_dist = XMVectorMultiply(half_diag, m_lightspace_view_nearside_scaling.V());
	r_camera_distance_from_frustum_centre = XMVectorAdd(m_lightspace_view_neardist.V(), near_side_dist);
	r_lightspace_view_fardist = XMVectorAdd(r_camera_distance_from_frustum_centre.V(), half_diag);
}


// Process the render queue and issue draw calls for all shadow-casting entities in range of the given light
void ShadowManagerComponent::ExecuteLightSpaceRenderPass(unsigned int light_index, const LightData & light)
{
	bool directional_light = (light.Type == LightType::Directional);
	const auto & renderqueue = Game::Engine->GetRenderQueue();
	
	// Activate the pipeline for the active light
	ActivateLightSpaceShadowmapPipeline(light);
	++m_active_shadow_map_count;

	// For each shader
	for (auto i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		const auto & shadermodels = renderqueue[i];
		size_t shadermodelcount = shadermodels.CurrentSlotCount;

		// For each model being rendered by the shader
		for (size_t modelix = 0U; modelix < shadermodelcount; ++modelix)
		{
			const auto & modeldata = shadermodels.ModelData[modelix];
			size_t materialcount = modeldata.CurrentMaterialCount;

			// For each material variation of this model
			for (size_t matix = 0; matix < materialcount; ++matix)
			{
				const auto & matdata = modeldata.Data[matix];

				// Skip if none of the instances are shadow-casters (or tf also if there are no instances)
				size_t shadowercount = matdata.InstanceCollection.ShadowCasterCount;
				if (shadowercount == 0U) continue;
				size_t instancecount = matdata.InstanceCollection.CurrentInstanceCount;

				// Debug assertion
				assert(instancecount < 2048U);
				assert(shadowercount < 2048U);
				assert(shadowercount <= instancecount);

				// Make sure there is enough space to hold all potential shadow-casters
				if (shadowercount > m_instance_capacity)
				{
					int required = static_cast<int>(shadowercount - m_instance_capacity);
					assert(required < 2048U);

					for (int i = 0; i < required; ++i) m_instances.push_back(std::move(RM_Instance()));
					m_instance_capacity = shadowercount;
				}

				// Collect the set of shadow-casters to be rendered, based on shadow caster state and light details
				size_t rendercount = 0U;
				const RM_Instance *instances = &(m_instances[0]);

				// Optimisation: if this is a directional light, and all instances are shadow-casters, we can use the actual instance vector directly
				if (directional_light && shadowercount == instancecount)
				{
					rendercount = shadowercount;
					instances = &(matdata.InstanceCollection.InstanceData[0]);
				}
				else
				{
					// Otherwise, we need to check each instance in turn
					for (size_t ix = 0U; ix < instancecount; ++ix)
					{
						const auto & inst = matdata.InstanceCollection.InstanceData[ix];
						
						if (!CheckBit_Single(inst.Flags, RM_Instance::INSTANCE_FLAG_SHADOW_CASTER)) continue;
						if (!InstanceIntersectsLightFrustum(matdata.InstanceCollection.InstanceMetadata[ix], light)) continue;

						m_instances[rendercount] = RM_Instance(inst);
						++rendercount;
					}
				}

				// Make sure we have some renderable instances
				if (rendercount == 0U) continue;

				// Submit the instances for rendering
				Game::Engine->RenderInstanced(*m_pipeline_lightspace_shadowmap, *modeldata.ModelBufferInstance, matdata.Material, *instances, static_cast<UINT>(rendercount));
			}
		}
	}

	// Perform a resource copy of this shadow map data if we have the debug capture enabled
#ifdef _DEBUG
	if (light_index == m_debug_shadowmap_capture_id) DebugCaptureActiveShadowMap();
#endif

	// Deactivate the pipeline and unbind all resources
	DeactivateLightSpaceShadowmapPipeline();

	// Populate any CB data required for rendering that uses this shadow map and compile the buffer ready for binding
	CompileShadowMappedLightDataBuffer(light);
}

// Activate the light-space shadow mapping pipeline for the given light object
void ShadowManagerComponent::ActivateLightSpaceShadowmapPipeline(const LightData & light)
{
	// Determine the shadow map configuration required for this light
	SMSize sm = static_cast<SMSize>(light.ShadowMapConfig);
	if (sm == SMSize::SMUnknown) sm = m_standard_shadow_map_size[static_cast<unsigned int>(light.Type)];

	// Determine view/projection and other light data for this light object based on its position in the world
	m_active_sm_light_position = XMLoadFloat4(&light.PositionWS);
	m_active_sm_light_orientation = QuaternionBetweenVectors(FORWARD_VECTOR, XMLoadFloat4(&light.DirectionWS));

	XMMATRIX view = LightViewMatrix(light);
	XMMATRIX proj = LightProjMatrix(light);
	m_active_sm_viewproj = XMMatrixMultiply(view, proj);
	
	// Populate the light-space SM data buffer based on this light
	XMStoreFloat4x4(&(m_cb_lightspace_shadowmap_data.RawPtr->LightViewProjection), m_active_sm_viewproj);
	m_cb_lightspace_shadowmap->Set(m_cb_lightspace_shadowmap_data.RawPtr);

	// Bind resources to the shadow mapping shader
	m_vs_lightspace_shadowmap->SetParameterData(m_param_vs_shadowmap_smdata, m_cb_lightspace_shadowmap);

	// Prepare the pipeline to render a shadow map with this configuration
	ActivateShadowMapConfiguration(sm);

	// Bind the pipeline ready for rendering
	m_pipeline_lightspace_shadowmap->Bind();
}

// Prepare the pipeline to render a shadow map with this configuration
void ShadowManagerComponent::ActivateShadowMapConfiguration(SMSize config)
{
	ShadowMapConfig & new_config = m_shadow_map_config[static_cast<unsigned int>(config)];

	// A pipeline update is only required if this is a change in config
	if (config != m_current_shadow_map_size)
	{
		m_pipeline_lightspace_shadowmap->SetRenderTarget(new_config.RenderTarget);
		m_pipeline_lightspace_shadowmap->GetRasterizerState().SetCompiledViewportDirect(
			new_config.Viewport, new_config.ViewportCompiled);

		m_current_shadow_map_size = config;
	}

	// In all cases, we need to clear the DSV ready for rendering
	m_shadow_map_config[static_cast<unsigned int>(config)].Texture->Clear(ClearFlags::Depth);
}

// Deactivate the light-space shadow mapping pipeline
void ShadowManagerComponent::DeactivateLightSpaceShadowmapPipeline(void)
{
	// Unbind the pipeline and all its shaders & resources
	m_pipeline_lightspace_shadowmap->Unbind();
}

// Store any required data on the active shadow map before returning
void ShadowManagerComponent::CompileShadowMappedLightDataBuffer(const LightData & light)
{
	// Calculate required data
	XMMATRIX cam_to_light_projection = XMMatrixMultiply(
		Game::Engine->GetRenderInverseViewProjectionMatrix(),		// InvCamProj * InvCamView
		m_active_sm_viewproj										// * LightView * LightProj
	);

	// Populate the buffer data
	XMStoreFloat4x4(&(m_cb_shadowmapped_light_data.RawPtr->CamToLightProjection), cam_to_light_projection);


	// Compile the buffer
	m_cb_shadowmapped_light->Set(m_cb_shadowmapped_light_data.RawPtr);
}

// Calculate transform matrix for the given light
XMMATRIX ShadowManagerComponent::LightViewMatrix(const LightData & light) const
{
	// TODO (SM): temp
	if (Game::Keyboard.GetKey(DIK_F) && Game::Keyboard.CtrlDown())
	{
		XMVECTOR dir = XMLoadFloat4(&light.DirectionWS);
		Game::Engine->GetCamera()->SetDebugCameraPosition(XMVectorMultiplyAdd(
			r_camera_distance_from_frustum_centre.V(),								// (Vectorised distance from camera to frustum centre)
			XMVectorNegate(dir),													// * -lightdir
			r_frustum_centre_point));
		Game::Engine->GetCamera()->SetDebugCameraOrientation(m_active_sm_light_orientation);
		Game::Log << LOG_DEBUG << "DBG pos = " << Vector4ToString(Game::Engine->GetCamera()->GetDebugCameraPosition()) <<
			", Orient = " << Vector4ToString(Game::Engine->GetCamera()->GetDebugCameraOrientation()) << "\n";
		Game::Keyboard.LockKey(DIK_F);
	}

	// TODO: It may be possible to do this more efficiently, e.g. setting matrix components directly?
	switch (light.Type)
	{
		case LightType::Directional:
			
			// Step back from view frustum centre by (lightdir * (frustum_size + NEAR))
			XMVECTOR dir = XMLoadFloat4(&light.DirectionWS);
			return CameraView::View(XMVectorMultiplyAdd(
				r_camera_distance_from_frustum_centre.V(),				// (Vectorised distance from camera to frustum centre)
				XMVectorNegate(dir),									// * -lightdir
				r_frustum_centre_point),								// + centre_point
				m_active_sm_light_orientation);

		case LightType::Spotlight:
			return CameraView::View(m_active_sm_light_position, m_active_sm_light_orientation);

		// Case Point, or as default
		default: 
			return CameraView::View(m_active_sm_light_position, ID_MATRIX);
	}
	
}

// Calculate transform matrix for the given light
// TODO: This can be cached and only recalculated on display/view plane/aspect/FOV change, if needed
XMMATRIX ShadowManagerComponent::LightProjMatrix(const LightData & light) const
{
	// TODO: near/far plane, FOV, and frustum in general will be calculated per light during cascaded shadow mapping
	const auto *rd = Game::Engine->GetRenderDevice();
	switch (light.Type)
	{
		case LightType::Directional:
			return CameraProjection::Orthographic(XMFLOAT2(r_frustum_longest_diagonal_mag.F(), r_frustum_longest_diagonal_mag.F()), 
				m_lightspace_view_neardist.F(), r_lightspace_view_fardist.F());

		// Case Point, Spotlight, or as default
		default:
			return CameraProjection::Perspective(rd->GetFOV(), rd->GetAspectRatio(), rd->GetNearClipDistance(), rd->GetFarClipDistance());
	}
}

// Determines whether a render instance intersects with the given light object
bool ShadowManagerComponent::InstanceIntersectsLightFrustum(const RM_InstanceMetadata & instance_metadata, const LightData & light)
{
	switch (light.Type)
	{
		// Point lights simply test bounding sphere against point light range
		case LightType::Point:
			return IntersectionTests::SphereSphere(instance_metadata.Position, instance_metadata.BoundingSphereRadius, XMLoadFloat4(&light.PositionWS), light.Range);

		// TODO: for now, simply test spotlights based upon range and ignore the spotlight direction/angle.  Fix this to correctly
		// limit objects passing the test
		case LightType::Spotlight:
			return IntersectionTests::SphereSphere(instance_metadata.Position, instance_metadata.BoundingSphereRadius, XMLoadFloat4(&light.PositionWS), light.Range);

		// Directional lights always intersect
		default:
			return true;
	}
}

std::string ShadowManagerComponent::ShadowmapTextureAssetName(SMSize size)
{
	return concat("ShadowMap_TX_")(static_cast<int>(size)).str();
}

std::string	ShadowManagerComponent::ShadowmapRenderTargetAssetName(SMSize size)
{
	return concat("ShadowMap_RT_")(static_cast<int>(size)).str();
}


// Debug capture of shadow map bufers for the given light
void ShadowManagerComponent::EnableShadowMapCapture(int light_index)
{
#ifdef _DEBUG
	m_debug_shadowmap_capture_id = light_index;
#else
	DisableShadowMapCapture();
#endif
}

// Debug capture of shadow map bufers for the given light
void ShadowManagerComponent::DisableShadowMapCapture(void)
{
	m_debug_shadowmap_capture_id = DEBUG_SHADOW_MAP_CAPTURE_DISABLED;
	DiscardDebugShadowMapCaptureBuffer();
}

// Disard the shadow map debug capture buffer
void ShadowManagerComponent::DiscardDebugShadowMapCaptureBuffer(void)
{
	if (m_debug_shadowmap_capture)
	{
		Game::Engine->GetAssets().DeleteTexture(TX_SHADOWMAP_DEBUG_CAPTURE);
		m_debug_shadowmap_capture = NULL;
		m_debug_shadowmap_capture_size = SMSize::SMUnknown;
	}
}

// Copy the active shadow map resource (debug mode only)
void ShadowManagerComponent::DebugCaptureActiveShadowMap(void)
{
	// If the capture buffer already exists, but does not match the current SM size, then dispose of it here so it can be recreated
	if (m_debug_shadowmap_capture && (m_debug_shadowmap_capture_size != m_current_shadow_map_size))
	{
		DiscardDebugShadowMapCaptureBuffer();
	}

	// We need to recreate the capture buffer if it doesn't exist
	if (!m_debug_shadowmap_capture)
	{
		// Initialise a new debug capture buffer if not already present
		unsigned int texsize = GetShadowMapSizeInTexels(m_current_shadow_map_size);
		m_debug_shadowmap_capture = Game::Engine->GetAssets().CreateTexture2D(TX_SHADOWMAP_DEBUG_CAPTURE,
			texsize, texsize, 1U, Game::Engine->GetRenderDevice()->PrimaryRenderTargetDepthStencilBufferFormat());

		// Terminate on allocation failure
		if (!m_debug_shadowmap_capture)
		{
			Game::Log << LOG_ERROR << "Error while trying to allocate new debug shadow map capture buffer; terminating capture";
			DisableShadowMapCapture();
			return;
		}

		m_debug_shadowmap_capture_size = m_current_shadow_map_size;
	}

	// Copy current shadowmap to the debug capture buffer
	m_debug_shadowmap_capture->Copy(GetActiveShadowMap());
}

// Return the debug capture of the active shadow map buffer
TextureDX11 * ShadowManagerComponent::GetDebugShadowMapBufferCapture(void)
{
	return m_debug_shadowmap_capture;
}

// Destructor
ShadowManagerComponent::~ShadowManagerComponent(void)
{

}


// Virtual inherited method to accept a command from the console
bool ShadowManagerComponent::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "shadow")
	{
		if (command.Parameter(0) == "map")
		{
			if (command.Parameter(1) == "status")
			{
				command.SetSuccessOutput(concat("Frame ")(Game::FrameIndex)(": ")(GetActiveShadowMapCount())(" shadow maps currently active").str());
				return true;
			}
			else if (command.Parameter(1) == "defaultsize")
			{
				if (command.ParameterCount() >= 4)
				{
					LightType light = static_cast<LightType>(command.ParameterAsInt(2));
					int isize = command.ParameterAsInt(3);

					if (!Light::IsValidLightType(light))	command.SetFailureOutput(concat("Cannot set default shadow map size; light type ")(static_cast<int>(light))(" is not valid").str());
					else if (isize <= 0)					command.SetFailureOutput(concat("Cannot set default shadow map size; size of ")(isize)(" is not valid").str());
					else
					{
						SMSize size = GetShadowMapSizeFromTexels(static_cast<unsigned int>(isize));
						if (size == SMSize::SMUnknown) command.SetFailureOutput(concat("Cannot set default shadow map size; ")(isize)(" is not a supported texel size").str());
						else
						{
							bool success = SetStandardShadowMapSize(light, size);
							if (!success) command.SetFailureOutput(concat("Failed to update default shadow map size for light=")(light)(" to ")(static_cast<int>(size))("-")(isize).str());
							else
							{
								command.SetSuccessOutput(concat("Default shadow map size for light=")(static_cast<int>(light))(" set to ")(static_cast<int>(size))("-")(isize).str());
							}
						}
					}
					return true;
				}
				else if (command.ParameterCount() == 3)
				{
					LightType light = static_cast<LightType>(command.ParameterAsInt(2));
					if (!Light::IsValidLightType(light)) command.SetFailureOutput(concat("Unrecognised light type ")(static_cast<int>(light)).str());
					else
					{
						unsigned int stdsize = GetStandardShadowMapTextureSize(light);
						command.SetSuccessOutput(concat("Default shadow map size for light=")(static_cast<int>(light))(" is ")(stdsize)("x")(stdsize).str());
					}
					return true;
				}
			}
			else if (command.Parameter(1) == "capture")
			{
				if (command.ParameterCount() >= 3 && command.ParameterAsInt(2) >= 0)
				{
					EnableShadowMapCapture(command.ParameterAsInt(2));
					command.SetSuccessOutput(concat("Enabling shadow map buffer capture for light index ")(command.ParameterAsInt(2)).str());
				}
				else
				{
					DisableShadowMapCapture();
					command.SetSuccessOutput("Disabling shadow map buffer capture");
				}
				return true;
			}
			else if (command.Parameter(1) == "nearscale")
			{
				if (command.ParameterCount() >= 3)
				{
					float scale = command.ParameterAsFloat(2);
					if (scale >= 0.0f && scale < 1000.0f) 
					{
						SetShadowMapNearSideFrustumScaling(scale);
						command.SetSuccessOutput(concat("Updated shadow map near-side frustum scaling factor to ")(scale).str());
					}
					else 
					{
						command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::InvalidShadowMapNearScaleFactor, 
							concat("Invalid shadow map near-side frustum scaling value: ")(scale).str());
					}
				}
				else
				{
					command.SetSuccessOutput(concat("Shadow map near-side frustum scaling factor = ")(GetShadowMapNearSideFrustumScaling()).str());
				}
				return true;
			}
		}
	}

	// Unrecognised command
	return false;
}




