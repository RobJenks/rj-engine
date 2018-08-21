#pragma once

#include <array>
#include <vector>
#include "VectorisedFloat.h"
#include "ManagedPtr.h"
#include "iAcceptsConsoleCommands.h"
#include "RM_Instance.h"
#include "RM_InstanceMetadata.h"
#include "ShaderDX11.h"
#include "ViewPort.h"
#include "../Definitions/LightData.hlsl.h"
#include "Data/Shaders/shadowmap_resources.hlsl"
class DeferredRenderProcess;
class TextureDX11;
class RenderTargetDX11;
class PipelineStateDX11;
class Frustum;
struct LightData;
struct GameConsoleCommand;

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
	void										ExecuteLightSpaceRenderPass(unsigned int light_index, const LightData & light);

	// Returns a pointer to the active shadow map buffer resource
	TextureDX11 *								GetActiveShadowMap(void);

	// Enumeration of supported shadow map sizes
	enum										SMSize { SMUnknown = 0, SM256, SM512, SM1024, SM2048, SM4096, _SMCOUNT };

	// Shadow map size
	unsigned int								GetShadowMapSizeInTexels(SMSize size) const;
	SMSize										GetShadowMapSizeFromTexels(unsigned int tex_size) const;

	SMSize										GetStandardShadowMapSize(LightType light_type) const;
	unsigned int								GetStandardShadowMapTextureSize(LightType light_type) const;
	bool										SetStandardShadowMapSize(LightType light_type, SMSize size);
	bool										SetStandardShadowMapTextureSize(LightType light_type, unsigned int size);
	bool										ResetStandardShadowMapSize(LightType light_type);
	void										ResetStandardShadowMapSizes(void);

	// Return the number of currently-active shadow maps (as of the last rendered frame)
	CMPINLINE unsigned int						GetActiveShadowMapCount(void) const { return m_active_shadow_map_count; }

	// Returns a pointer to the compiled data buffer for the active SM, for use in the primary render pipeline
	CMPINLINE ConstantBufferDX11 *				GetShadowMappedLightDataBuffer(void) const { return m_cb_shadowmapped_light; }

	// Destructor
	~ShadowManagerComponent(void);

	// Debug capture of shadow map bufers for the given light
	void										EnableShadowMapCapture(int light_index);
	void										DisableShadowMapCapture(void);
	void										DiscardDebugShadowMapCaptureBuffer(void);
	TextureDX11 *								GetDebugShadowMapBufferCapture(void);


	// Virtual inherited method to accept a command from the console
	bool										ProcessConsoleCommand(GameConsoleCommand & command);

private:

	// Initialisation
	void										InitialiseShadowMapConfigurations(void);
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

	// Prepare the pipeline to render a shadow map with this configuration
	void										ActivateShadowMapConfiguration(SMSize config);

	// Populate the shadow mapped light CB for use in the primary rendering pipeline
	void										CompileShadowMappedLightDataBuffer(const LightData & light);

	// Calculate transform matrix for the given light
	XMMATRIX									LightViewMatrix(const LightData & light) const;
	XMMATRIX									LightProjMatrix(const LightData & light) const;

	// Retrieve 

	// Shadow map near-side frustum scaling factor (1.0 = near plane at outer extent of camera view frustum, 
	// 0.0 would have the near plane all the way forward at the camera view frustum centre point)
	CMPINLINE float								GetShadowMapNearSideFrustumScaling(void) const { return m_lightspace_view_nearside_scaling.F(); }
	CMPINLINE void								SetShadowMapNearSideFrustumScaling(float scale) { m_lightspace_view_nearside_scaling = scale; }

	// Copy the active shadow map resource (debug mode only)
	void										DebugCaptureActiveShadowMap(void);


private:

	struct ShadowMapConfig
	{
		SMSize									Size;
		unsigned int							TexelSize;
		TextureDX11 *							Texture;
		RenderTargetDX11 *						RenderTarget;
		Viewport								Viewport;
		D3D11_VIEWPORT							ViewportCompiled;
	};


private:

	DeferredRenderProcess *						m_renderprocess;

	const Frustum *								r_viewfrustum;							// Relevant for the current frame only
	XMVECTOR									r_frustum_world_corners[8];				// Relevant for the current frame only
	XMVECTOR									r_frustum_world_min;					// Relevant for the current frame only
	XMVECTOR									r_frustum_world_max;					// Relevant for the current frame only
	VectorisedFloat								r_frustum_longest_diagonal_mag;			// Relevant for the current frame only
	XMVECTOR									r_frustum_centre_point;					// Relevant for the current frame only
	VectorisedFloat								r_lightspace_view_fardist;				// Relevant for the current frame only
	VectorisedFloat								r_camera_distance_from_frustum_centre;	// Relevant for the current frame only

	
	// All supported shadow map configurations
	std::array<ShadowMapConfig, static_cast<int>(SMSize::_SMCOUNT)>		m_shadow_map_config;		

	// The active shadow map, or SMUnknown if no shadow map is enabled
	SMSize										m_current_shadow_map_size;

	static const SMSize							DEFAULT_SHADOW_MAP_SIZE[static_cast<int>(LightType::_LightTypeCount)];		// Compiled default per light type
	SMSize										m_standard_shadow_map_size[static_cast<int>(LightType::_LightTypeCount)];	// Current default per light type

	
	static const float							DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_DIST;
	VectorisedFloat								m_lightspace_view_neardist;

	static const float							DEFAULT_LIGHT_SPACE_FRUSTUM_NEAR_SIDE_SCALING;
	VectorisedFloat								m_lightspace_view_nearside_scaling;

	ShaderDX11 *								m_vs_lightspace_shadowmap;
	PipelineStateDX11 *							m_pipeline_lightspace_shadowmap;

	// CB input to the shadow mapping process
	ManagedPtr<LightSpaceShadowMapDataBuffer>	m_cb_lightspace_shadowmap_data;
	ConstantBufferDX11 *						m_cb_lightspace_shadowmap;

	// CB input to the primary rendering process, containing data that was calculated during the shadow mapping process
	ManagedPtr<ShadowMappedLightBuffer>			m_cb_shadowmapped_light_data;
	ConstantBufferDX11 *						m_cb_shadowmapped_light;

	// Data on the current shadow mapping render, i.e. the last light to be shadow-mapped
	XMVECTOR									m_active_sm_light_position;		// Position of the current light in world space
	XMVECTOR									m_active_sm_light_orientation;	// Orientation of the current light in world space
	XMMATRIX									m_active_sm_viewproj;			// View-projection for the current light

	ShaderDX11::ShaderParameterIndex			m_param_vs_shadowmap_smdata;

#ifdef _DEBUG
	TextureDX11 *								m_debug_shadowmap_capture;
#endif

	unsigned int								m_active_shadow_map_count;

	static const unsigned int					DEBUG_SHADOW_MAP_CAPTURE_DISABLED = (std::numeric_limits<unsigned int>::max)();
	unsigned int								m_debug_shadowmap_capture_id;
	SMSize										m_debug_shadowmap_capture_size;

	// Persistent vector used to hold instance data for light-space rendering.  Maintain as persistent storage
	// to avoid repeated reallocations between frames
	std::vector<RM_Instance>							m_instances;			// Vector of instance data
	std::vector<RM_Instance>::size_type					m_instance_capacity;	// Number of elements already allocated in vector
	static const std::vector<RM_Instance>::size_type	INITIAL_INSTANCE_DATA_CAPACITY = 64U;

private:

	static std::string							ShadowmapTextureAssetName(SMSize size);
	static std::string							ShadowmapRenderTargetAssetName(SMSize size);
	static const std::string					TX_SHADOWMAP_DEBUG_CAPTURE;
	static const std::string					RP_SHADOWMAP;

};