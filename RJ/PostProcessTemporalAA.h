#pragma once

#include <vector>
#include "DX11_Core.h"
#include "PostProcessComponent.h"
#include "ManagedPtr.h"
#include "ShaderDX11.h"
#include "Data/Shaders/temporal_aa_resources.hlsl"
class DeferredRenderProcess;
class TextureDX11;
class RenderTargetDX11;
class PipelineStateDX11;
class ConstantBufferDX11;


class PostProcessTemporalAA : public PostProcessComponent
{
public:

	// Initialise the postprocess and all required resources
	PostProcessTemporalAA(void);
	PostProcessTemporalAA(DeferredRenderProcess * render_process);

	// Reinitialise based on a change to the effect configuration
	void								PerformPostConfigInitialisation(void);

	// Execute the post-process over the source buffer.  Returns a pointer to the final buffer
	// following post-processing
	TextureDX11 *						Execute(TextureDX11 *source_colour, TextureDX11 *source_depth, TextureDX11 *source_vel, TextureDX11 *source_motion);

	// Virtual event which can be handled by subclasses; triggered when component is activated or deactivated
	void								ActiveStateChanged(bool is_active);

	// Respond to a change in shader configuration or a reload of shader bytecode
	void								ShadersReloaded(void);


	// Destructor
	~PostProcessTemporalAA(void);

private:

	// Initialisation
	void InitialiseShaders(void);
	void InitialiseTextureBuffers(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);
	void InitialiseTemporalPipeline(void);

	// Execution
	void PopulateTemporalAABuffer(void);
	void PrepareReprojectionBuffers(TextureDX11 *source_colour);
	void ExecuteTemporalReprojectionPass(TextureDX11 *source_colour, TextureDX11 *source_depth, TextureDX11 *source_vel, TextureDX11 *source_motion);

	// Reset any frame-dependent or temporal parameters, e.g. if the component is being activated
	void Reset(void);


private:

	static const std::string TX_NAME_REPROJECTION0;
	static const std::string TX_NAME_REPROJECTION1;
	static const std::string TX_NAME_FINAL;
	static const std::string RT_NAME_0;
	static const std::string RT_NAME_1;
	static const std::string RP_TEMPORAL;

	DeferredRenderProcess * m_renderprocess;

	ShaderDX11 * m_vs_quad;
	ShaderDX11 * m_ps_temporal;

	TextureDX11 * m_tx_reprojection[2];			// Maintain two reprojection buffers so we can read from the history buffer & write to the next one in parallel without unbinding
	TextureDX11 * m_tx_final;					// Final colour buffer output 

	RenderTargetDX11 *	m_rt[2];				// Maintain two render targets for efficiency: { { reproj[0], final }, { reproj[1], final } }

	PipelineStateDX11 * m_pipeline_temporal;	// Primary pipeline for temporal reprojection

	ManagedPtr<TemporalAABuffer>		m_cb_temporal_data;
	ConstantBufferDX11 *				m_cb_temporal;

	static const int					INITIAL_REPROJECTION_INDEX = -1;
	int									m_reprojection_index;
	int									m_frame_reprojection_source;		// Reprojection source for this frame
	int									m_frame_reprojection_target;		// Reprojection target for this frame

	static const float					DEFAULT_TEMPORAL_FEEDBACK_MIN;
	static const float					DEFAULT_TEMPORAL_FEEDBACK_MAX;
	float								m_temporal_feedback_min;
	float								m_temporal_feedback_max;

	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_deferred;
	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_buffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_tex_colour_buffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_tex_reproj_buffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_tex_depth_buffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_tex_velocity_buffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_tex_motion_buffer;
};