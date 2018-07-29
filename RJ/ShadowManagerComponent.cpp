#include "ShadowManagerComponent.h"
#include "LightData.hlsl.h"
#include "CoreEngine.h"
#include "RenderProcessDX11.h"
#include "RenderQueue.h"
#include "RM_ModelDataCollection.h"
#include "RM_MaterialInstanceMapping.h"
#include "RM_InstanceData.h"
#include "IntersectionTests.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "ShaderDX11.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "PipelineStateDX11.h"

// Initialise static data
const std::string ShadowManagerComponent::TX_SHADOWMAP = "ShadowMap_TX";
const std::string ShadowManagerComponent::RT_SHADOWMAP = "ShadowMap_RT";
const std::string ShadowManagerComponent::RP_SHADOWMAP = "ShadowMap_Pipeline";



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
	m_shadowmap_tx(NULL), 
	m_shadowmap_rt(NULL), 
	m_pipeline_lightspace_shadowmap(NULL), 

	m_param_vs_shadowmap_framebuffer(ShaderDX11::INVALID_SHADER_PARAMETER), 

	m_instance_capacity(0U)
{
	InitialiseShaders();
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

void ShadowManagerComponent::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Intialising shadow manager shaders\n";

	// Get a reference to all required shaders
	m_vs_lightspace_shadowmap = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::ShadowMappingVertexShader);
	if (m_vs_lightspace_shadowmap == NULL) Game::Log << LOG_ERROR << "Cannot load light-space shadow mapping shader resources [vs]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_shadowmap_framebuffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_vs_lightspace_shadowmap, FrameDataBufferName);
}


void ShadowManagerComponent::InitialiseTextureBuffers(void)
{
	Texture::TextureFormat & primary_colour_buffer_format = Game::Engine->GetRenderDevice()->PrimaryRenderTargetColourBufferFormat();
	Texture::TextureFormat & primary_dsv_buffer_format = Game::Engine->GetRenderDevice()->PrimaryRenderTargetDepthStencilBufferFormat();
	IntegralVector2<unsigned int> display_size = Game::Engine->GetRenderDevice()->GetDisplaySizeU();

	// Texture resource details
	std::vector<std::tuple<const std::string, const std::string, TextureDX11**, Texture::TextureFormat&, IntegralVector2<unsigned int>>> textures = {
		{ "light-space DSV", TX_SHADOWMAP, &m_shadowmap_tx, primary_dsv_buffer_format, display_size },	// TODO: can probably reduce from DSV to depth-only in future
	};

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
	IntegralVector2<unsigned int> display_size = Game::Engine->GetRenderDevice()->GetDisplaySizeU();

	// Render target data (desc, name, pRT, size, colour-buffer, dsv-buffer)
	std::vector<std::tuple<std::string, std::string, RenderTargetDX11**, IntegralVector2<int>, TextureDX11*, TextureDX11*>> rts = {
		{ "light-space render target", RT_SHADOWMAP, &m_shadowmap_rt, display_size.Convert<int>(), NULL, m_shadowmap_tx }
	};

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
	m_pipeline_lightspace_shadowmap->SetRenderTarget(m_shadowmap_rt);

	// Standard depth buffer operations; we want to render a closest-depth buffer from the light perspective
	DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Enable, DepthStencilState::CompareFunction::Less);
	m_pipeline_lightspace_shadowmap->GetDepthStencilState().SetDepthMode(depthMode);
	m_pipeline_lightspace_shadowmap->GetRasterizerState().SetViewport(Game::Engine->GetRenderDevice()->GetPrimaryViewport());

	// Front-face culling so that backfaces are shadowed; reduces artifacts when compared to back-face culling
	m_pipeline_lightspace_shadowmap->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Front);

	// Disable all blending operations in the target buffer; this is depth-only anyway so won't have any effect
	m_pipeline_lightspace_shadowmap->GetBlendState().SetBlendMode(BlendState::BlendModes::NoBlend);
}

void ShadowManagerComponent::InitialiseRenderQueueProcessing(void)
{
	Game::Log << LOG_INFO << "Intialising shadow manager render queue processing\n";

	// Preallocate space in the instance data collection
	m_instance_capacity = INITIAL_INSTANCE_DATA_CAPACITY; 
	m_instances.reserve(INITIAL_INSTANCE_DATA_CAPACITY);
	for (int i = 0; i < INITIAL_INSTANCE_DATA_CAPACITY; ++i)
	{
		m_instances.push_back(std::move(RM_Instance()));
	}	
}




// Process the render queue and issue draw calls for all shadow-casting entities in range of the given light
void ShadowManagerComponent::ExecuteLightSpaceRenderPass(const LightData & light)
{
	bool directional_light = (light.Type == LightType::Directional);
	const auto & renderqueue = Game::Engine->GetRenderQueue();
	
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

// Destructor
ShadowManagerComponent::~ShadowManagerComponent(void)
{

}
