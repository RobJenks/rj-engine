#pragma once

#include "RenderProcessDX11.h"
#include "ManagedPtr.h"
#include "Data/Shaders/BasicTextureRenderingCommonData.hlsl.h"
class PipelineStateDX11;
class RenderTargetDX11;
class ConstantBufferDX11;
class ShaderDX11;
class Model;


class UIRenderProcess : public RenderProcessDX11
{
public:

	// Constructor
	UIRenderProcess(void);

	// Virtual render method; must be implemented by all derived render processess
	void							Render(void);

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	void							PerformPostDataLoadInitialisation(void);


private:

	void							InitialiseShaders(void);
	void							InitialiseStandardBuffers(void);
	void							InitialiseRenderGeometry(void);
	void 							InitialisePipelines(void);

	void							PopulateFrameBuffer(void);

	CMPINLINE ConstantBufferDX11 *	GetFrameDataBuffer(void) { return m_cb_frame; }

private:

	// Shaders
	ShaderDX11 *						m_vs;
	ShaderDX11 *						m_ps;

	// Render pipeline for standard orthographic texture rendering
	PipelineStateDX11 *					m_pipeline;

	// Frame data buffer
	ManagedPtr<BasicTextureRenderingFrameBuffer>	m_cb_frame_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *							m_cb_frame;					// Compiled CB

	// Pre-cached models for orthographic screen rendering
	Model *								m_model_quad;

	// Indices of required shader parameters
	ShaderDX11::ShaderParameterIndex	m_param_vs_framedata;


};







