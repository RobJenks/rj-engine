#pragma once

#include <vector>
#include "RM_Instance.h"
#include "RM_InstanceMetadata.h"
class DeferredRenderProcess;
class PipelineStateDX11;
struct LightData;

class ShadowManagerComponent
{
public:

	// Constructor
	ShadowManagerComponent(void);
	ShadowManagerComponent(DeferredRenderProcess *renderprocess);

	// Initialisation of any components dependent on config load, or which should update on device parameter changes
	void										PerformPostConfigInitialisation(void);


	// Process the render queue and issue draw calls for all shadow-casting entities in range of the given light
	void										ExecuteLightSpaceRenderPass(const LightData & light);

	// Destructor
	~ShadowManagerComponent(void);


private:

	// Initialisation
	void										InitialiseShaders(void);
	void										InitialiseRenderPipelines(void);
	void										InitialiseRenderQueueProcessing(void);

	// Determines whether a render instance intersects with the given light object
	bool										InstanceIntersectsLightFrustum(const RM_InstanceMetadata & instance_metadata, const LightData & light);

private:

	DeferredRenderProcess *						m_renderprocess;

	PipelineStateDX11 *							m_lightspace_shadowmap_pipeline;


	// Persistent vector used to hold instance data for light-space rendering.  Maintain as persistent storage
	// to avoid repeated reallocations between frames
	std::vector<RM_Instance>							m_instances;			// Vector of instance data
	std::vector<RM_Instance>::size_type					m_instance_capacity;	// Number of elements already allocated in vector
	static const std::vector<RM_Instance>::size_type	INITIAL_INSTANCE_DATA_CAPACITY = 64U;

};