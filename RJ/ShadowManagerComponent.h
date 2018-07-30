#pragma once

#include <vector>
#include "ManagedPtr.h"
#include "RM_Instance.h"
#include "RM_InstanceMetadata.h"
#include "ShaderDX11.h"
#include "Data/Shaders/shadowmap_resources.hlsl"
class DeferredRenderProcess;
class TextureDX11;
class RenderTargetDX11;
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
	void										InitialiseStandardBuffers(void);
	void										InitialiseTextureBuffers(void);
	void										InitialiseRenderTargets(void);
	void										InitialiseRenderPipelines(void);
	void										InitialiseLightSpaceShadowMappingPipeline(void);
	void										InitialiseRenderQueueProcessing(void);

	// Determines whether a render instance intersects with the given light object
	bool										InstanceIntersectsLightFrustum(const RM_InstanceMetadata & instance_metadata, const LightData & light);

	// Activate or deactivate the light-space shadow mapping pipeline for a given light object
	void										ActivateLightSpaceShadowmapPipeline(const LightData & light);
	void										DeactivateLightSpaceShadowmapPipeline(void);

	// Calculate transform matrix for the given light
	XMMATRIX									LightViewMatrix(const LightData & light) const;
	XMMATRIX									LightProjMatrix(const LightData & light) const;

private:

	DeferredRenderProcess *						m_renderprocess;

	ShaderDX11 *								m_vs_lightspace_shadowmap;
	TextureDX11 *								m_shadowmap_tx;
	RenderTargetDX11 *							m_shadowmap_rt;
	PipelineStateDX11 *							m_pipeline_lightspace_shadowmap;

	ManagedPtr<LightSpaceShadowMapDataBuffer>	m_cb_lightspace_shadowmap_data;
	ConstantBufferDX11 *						m_cb_lightspace_shadowmap;

	ShaderDX11::ShaderParameterIndex			m_param_vs_shadowmap_smdata;


	// Persistent vector used to hold instance data for light-space rendering.  Maintain as persistent storage
	// to avoid repeated reallocations between frames
	std::vector<RM_Instance>							m_instances;			// Vector of instance data
	std::vector<RM_Instance>::size_type					m_instance_capacity;	// Number of elements already allocated in vector
	static const std::vector<RM_Instance>::size_type	INITIAL_INSTANCE_DATA_CAPACITY = 64U;

private:

	static const std::string					TX_SHADOWMAP;
	static const std::string					RT_SHADOWMAP;
	static const std::string					RP_SHADOWMAP;

};