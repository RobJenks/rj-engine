#pragma once

#include <vector>
#include "ManagedPtr.h"
#include "iAcceptsConsoleCommands.h"
#include "RM_Instance.h"
#include "RM_InstanceMetadata.h"
#include "ShaderDX11.h"
#include "Data/Shaders/shadowmap_resources.hlsl"
class DeferredRenderProcess;
class TextureDX11;
class RenderTargetDX11;
class PipelineStateDX11;
class Frustum;
struct LightData;
class GameConsoleCommand;

class ShadowManagerComponent : public iAcceptsConsoleCommands
{
public:

	// Constructor
	ShadowManagerComponent(void);
	ShadowManagerComponent(DeferredRenderProcess *renderprocess);

	// Initialisation of any components dependent on config load, or which should update on device parameter changes
	void										PerformPostConfigInitialisation(void);

	// Perform per-frame initialisation before any shadow map rendering is performed
	void										BeginFrame(void);

	// Process the render queue and issue draw calls for all shadow-casting entities in range of the given light
	void										ExecuteLightSpaceRenderPass(const LightData & light);

	// Size of all shadow maps (TODO: in future, allow customised size per SM, based on e.g. light?  Probably
	// not necessary if cascaded shadow mapping is adopted)
	CMPINLINE UINTVECTOR2						GetShadowMapSize(void) const { return m_shadow_map_size; }
	bool										SetShadowMapSize(UINTVECTOR2 size);

	// Return the number of currently-active shadow maps (as of the last rendered frame)
	CMPINLINE unsigned int						GetActiveShadowMapCount(void) const { return m_active_shadow_maps; }

	// Destructor
	~ShadowManagerComponent(void);

	// Virtual inherited method to accept a command from the console
	bool										ProcessConsoleCommand(GameConsoleCommand & command);

private:

	// Initialisation
	void										InitialiseShaders(void);
	void										InitialiseStandardBuffers(void);
	void										InitialiseTextureBuffers(void);
	void										InitialiseRenderTargets(void);
	void										InitialiseRenderPipelines(void);
	void										InitialiseLightSpaceShadowMappingPipeline(void);
	void										InitialiseRenderQueueProcessing(void);

	// Perform per-frame calculations based on the current view frustum
	void										CalculateViewFrustumData(const Frustum *frustum);

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

	const Frustum *								r_viewfrustum;						// Relevant for the current frame only
	XMVECTOR									r_frustum_world_corners[8];			// Relevant for the current frame only
	XMVECTOR									r_frustum_world_min;				// Relevant for the current frame only
	XMVECTOR									r_frustum_world_max;				// Relevant for the current frame only
	float										r_frustum_longest_diagonal_mag;		// Relevant for the current frame only
	XMVECTOR									r_frustum_longest_diagonal_mag_v;	// Relevant for the current frame only
	XMVECTOR									r_frustum_centre_point;				// Relevant for the current frame only
	float										r_lightspace_view_fardist;			// Relevant for the current frame only

	static const UINTVECTOR2					DEFAULT_SHADOW_MAP_SIZE;
	static const unsigned int					MAX_SHADOW_MAP_SIZE = 8192U;
	UINTVECTOR2									m_shadow_map_size;

	static const float							DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_DIST;
	float										m_lightspace_view_neardist;
	XMVECTOR									m_lightspace_view_neardist_v;

	ShaderDX11 *								m_vs_lightspace_shadowmap;
	TextureDX11 *								m_shadowmap_tx;
	RenderTargetDX11 *							m_shadowmap_rt;
	PipelineStateDX11 *							m_pipeline_lightspace_shadowmap;

	ManagedPtr<LightSpaceShadowMapDataBuffer>	m_cb_lightspace_shadowmap_data;
	ConstantBufferDX11 *						m_cb_lightspace_shadowmap;

	ShaderDX11::ShaderParameterIndex			m_param_vs_shadowmap_smdata;

	unsigned int								m_active_shadow_maps;

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