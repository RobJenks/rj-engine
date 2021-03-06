#pragma once

#include "RenderProcessDX11.h"
#include "ManagedPtr.h"
#include "Data/Shaders/SDFDecalRenderingCommonData.hlsl.h"
class Model;
class MaterialDX11;
class PipelineStateDX11;
class DecalRenderingParams;

class SDFDecalRenderProcess : public RenderProcessDX11
{

public:

	// Constructor
	SDFDecalRenderProcess(void);

	// Virtual render method; must be implemented by all derived render processess
	void							Render(void);

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	void							PerformPostDataLoadInitialisation(void);


private:

	enum class						FrameBufferMode { Uninitialised = 0, ScreenSpace, WorldSpace };

	void							InitialiseShaders(void);
	void							InitialiseStandardBuffers(void);
	void							InitialiseStandardMaterials(void);
	void							InitialiseRenderGeometry(void);
	void 							InitialisePipelines(void);
	void							InitialiseShaderResourceBindings(void);

	void							PopulateFrameDataBuffer(FrameBufferMode frame_buffer_mode);
	void							PopulateDecalDataBuffer(const DecalRenderingParams & render_group);

	CMPINLINE ConstantBufferDX11 *	GetFrameDataBuffer(void) { return m_cb_frame; }
	CMPINLINE ConstantBufferDX11 *	GetDecalRenderingConstantBuffer(void) { return m_cb_decal; }

	// Process a render group through the specified pipeline
	void							ExecuteRenderingPipeline(PipelineStateDX11 * pipeline, Model & geometry, const DecalRenderingParams & render_group);

private:

	// Shaders
	ShaderDX11 *							m_vs_direct;
	ShaderDX11 *							m_vs_deferred;
	ShaderDX11 *							m_ps_direct;
	ShaderDX11 *							m_ps_deferred;

	// Render pipelines for each supported decal rendering mode
	PipelineStateDX11 *						m_pipeline_direct;
	PipelineStateDX11 *						m_pipeline_deferredproj;

	// Frame data buffer
	FrameBufferMode							m_cb_frame_mode;
	ManagedPtr<DecalRenderingFrameBuffer>	m_cb_frame_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_frame;					// Compiled CB

	ManagedPtr<DecalRenderingDataBuffer>	m_cb_decal_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_decal;					// Compiled CB

	// Pre-cached models for orthographic screen rendering
	Model *									m_model_quad;
	Model *									m_model_cube;

	// Single material updated and used for rendering of all decals
	ManagedPtr<MaterialDX11>				m_decal_material;

	// Indices of required shader parameters
	ShaderDX11::ShaderParameterIndex		m_param_vs_direct_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_vs_deferred_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_deferred_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_direct_decaldata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_deferred_decaldata;




};