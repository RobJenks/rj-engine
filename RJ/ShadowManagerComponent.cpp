#include "ShadowManagerComponent.h"
#include "LightData.hlsl.h"
#include "CoreEngine.h"
#include "RenderQueue.h"
#include "RM_ModelDataCollection.h"
#include "RM_MaterialInstanceMapping.h"
#include "RM_InstanceData.h"
#include "IntersectionTests.h"


// Constructor
ShadowManagerComponent::ShadowManagerComponent(void)
{
	// Only for default intialisation
}

// Constructor
ShadowManagerComponent::ShadowManagerComponent(DeferredRenderProcess *renderprocess)
	:
	m_renderprocess(renderprocess), 

	m_lightspace_shadowmap_pipeline(NULL), 

	m_instance_capacity(0U)
{
	InitialiseShaders();
	InitialiseRenderPipelines();
	InitialiseRenderQueueProcessing();

	PerformPostConfigInitialisation();
}

void ShadowManagerComponent::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Intialising shadow manager shaders\n";

}

void ShadowManagerComponent::InitialiseRenderPipelines(void)
{
	Game::Log << LOG_INFO << "Intialising shadow manager render pipelines\n";

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

// Initialisation of all components which are config-dependent or should be refreshed on device parameters change (e.g. display size)
void ShadowManagerComponent::PerformPostConfigInitialisation(void)
{

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
				Game::Engine->RenderInstanced(*m_lightspace_shadowmap_pipeline, *modeldata.ModelBufferInstance, matdata.Material, *instances, static_cast<UINT>(rendercount));
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
