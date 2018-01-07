#include "DeferredRenderProcess.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "PipelineStateDX11.h"
#include "ShaderDX11.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Data\Shaders\Common\CommonShaderConstantBufferDefinitions.hlsl.h"


DeferredRenderProcess::DeferredRenderProcess(void)
	:
	m_vs(NULL), 
	m_ps_geometry(NULL), 
	m_ps_lighting(NULL), 
	m_depth_only_rt(NULL), 
	m_cb_frame(NULL), 
	m_cb_material(NULL), 

	m_pipeline_geometry(NULL), 
	m_pipeline_lighting_pass1(NULL), 
	m_pipeline_lighting_pass2(NULL), 
	m_pipeline_lighting_directional(NULL), 
	m_pipeline_transparency(NULL), 

	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_geom_framedata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_framedata(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_geom_materialdata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_materialdata(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	InitialiseShaders();
	InitialiseRenderTargets();
	InitialiseStandardBuffers();

	InitialiseGeometryPipelines();
	InitialiseDeferredLightingPipelines();
	InitialiseTransparentRenderingPipelines();
}

void DeferredRenderProcess::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering shaders\n";

	// Get a reference to all required shaders
	m_vs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::StandardPixelShader);
	if (m_vs == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [vs]\n";

	m_ps_geometry = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredGeometryPixelShader);
	if (m_ps_geometry == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_g]\n";

	m_ps_lighting = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredLightingPixelShader);
	if (m_ps_lighting == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_l]\n";

	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framedata = AttemptRetrievalOfShaderParameter(m_vs, FrameDataBufferName);
	m_param_ps_geom_framedata = AttemptRetrievalOfShaderParameter(m_ps_geometry, FrameDataBufferName);
	m_param_ps_geom_materialdata = AttemptRetrievalOfShaderParameter(m_ps_geometry, MaterialBufferName);
	m_param_ps_light_framedata = AttemptRetrievalOfShaderParameter(m_ps_lighting, FrameDataBufferName);
	m_param_ps_light_materialdata = AttemptRetrievalOfShaderParameter(m_ps_lighting, MaterialBufferName);

}

void DeferredRenderProcess::InitialiseRenderTargets(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering render targets\n";

	// Depth-only render target will be attached to the primary RT depth/stencil buffer
	m_depth_only_rt = Game::Engine->GetRenderDevice()->Assets.CreateRenderTarget("Deferred_DepthOnly");
	m_depth_only_rt->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil,
		Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->GetTexture(RenderTarget::AttachmentPoint::DepthStencil));
}

void DeferredRenderProcess::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialise deferred rendering standard buffer resources\n";

	m_cb_frame_data = { 0 };
	m_cb_material_data = { 0 };

	m_cb_frame = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<FrameDataBuffer>(FrameDataBufferName, m_cb_frame_data.RawPtr);
	m_cb_material = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<MaterialBuffer>(MaterialBufferName, m_cb_material_data.RawPtr);
}

// Geometry pipeline will render all opaque geomeetry to the GBuffer RT
void DeferredRenderProcess::InitialiseGeometryPipelines(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [g]\n";

	m_pipeline_geometry = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Geometry");
	m_pipeline_geometry->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_geometry->SetShader(Shader::Type::PixelShader, m_ps_geometry);
	m_pipeline_geometry->SetRenderTarget(GBuffer.RenderTarget);
}

// Multi-stage deferred lighting passes
void DeferredRenderProcess::InitialiseDeferredLightingPipelines(void)
{
	InitialiseDeferredLightingPass1Pipeline();
	InitialiseDeferredLightingPass2Pipeline();
}

// Lighting pass 1: determine lit pixels
void DeferredRenderProcess::InitialiseDeferredLightingPass1Pipeline(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [l1]\n";

	// First pass will only render depth information to an off-screen buffer
	m_pipeline_lighting_pass1 = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Lighting_Pass1");
	m_pipeline_lighting_pass1->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_lighting_pass1->SetRenderTarget(m_depth_only_rt);

	// Perform culling of back faces
	m_pipeline_lighting_pass1->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Back);
	m_pipeline_lighting_pass1->GetRasterizerState().SetDepthClipEnabled(true);

	// Enable depth testing (pass if light is behind/greater than geometry depth), disable depth writes
	DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Greater);
	m_pipeline_lighting_pass1->GetDepthStencilState().SetDepthMode(depthMode);

	// Enable stencil operations, decrement 1 on pass
	DepthStencilState::StencilMode stencilMode(true);
	DepthStencilState::FaceOperation faceOperation;
	faceOperation.StencilDepthPass = DepthStencilState::StencilOperation::DecrementClamp;
	stencilMode.StencilReference = 1U;
	stencilMode.FrontFace = faceOperation;
	m_pipeline_lighting_pass1->GetDepthStencilState().SetStencilMode(stencilMode);
}

// Lighting pass 2: render lit pixels
void DeferredRenderProcess::InitialiseDeferredLightingPass2Pipeline(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [l2]\n";

	// Second pass will render lighting information to the primary RT itself
	m_pipeline_lighting_pass2 = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Lighting_Pass2");
	m_pipeline_lighting_pass2->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_lighting_pass2->SetShader(Shader::Type::PixelShader, m_ps_lighting);
	m_pipeline_lighting_pass2->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());

	// Perform culling of front faces since we want to render only back faces of the light volume
	m_pipeline_lighting_pass2->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Front);
	m_pipeline_lighting_pass2->GetRasterizerState().SetDepthClipEnabled(false);

	// All light rendering will be additive
	m_pipeline_lighting_pass2->GetBlendState().SetBlendMode(BlendState::BlendModes::AdditiveBlend);

	// Enable depth testing (pass if in front of / greater than light volume back faces), disable depth writes
	DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::GreaterOrEqual);
	m_pipeline_lighting_pass2->GetDepthStencilState().SetDepthMode(depthMode);

	// Enable stencil operations, keep on pass ( == 1, i.e. not decremented/unmarked in pass1)
	DepthStencilState::StencilMode stencilMode(true);
	DepthStencilState::FaceOperation faceOperation;
	faceOperation.StencilFunction = DepthStencilState::CompareFunction::Equal;
	stencilMode.StencilReference = 1U;
	stencilMode.BackFace = faceOperation;
	m_pipeline_lighting_pass2->GetDepthStencilState().SetStencilMode(stencilMode);
}

// Directional lights can be rendered in a single pass, rather than the default two-pass deferred lighting calculation
void DeferredRenderProcess::InitialiseDeferredDirectionalLightingPipeline(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [ld]\n";

	// Directional lighting pass will use a single full-screen quad at the far clip plane.  All pixels 
	// forward of the plane will be lit using the same deferred lighting PS
	m_pipeline_lighting_directional = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Lighting_Directional");
	m_pipeline_lighting_directional->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_lighting_directional->SetShader(Shader::Type::PixelShader, m_ps_lighting);
	m_pipeline_lighting_directional->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());
	m_pipeline_lighting_directional->GetBlendState().SetBlendMode(BlendState::BlendModes::AdditiveBlend);

	// Enable depth testing (pass all pixels in front of the far plane), disable writes to the depth buffer
	DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Greater);
	m_pipeline_lighting_directional->GetDepthStencilState().SetDepthMode(depthMode);
}

void DeferredRenderProcess::InitialiseTransparentRenderingPipelines(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [t]\n";

	// Use a standard pipeline state here; nothing specific to deferred rendering
	m_pipeline_transparency = Game::Engine->GetRenderDevice()->Assets.GetPipelineState("Transparency");
	if (!m_pipeline_transparency)
	{
		Game::Log << LOG_ERROR << "Could not initialise deferred lighting transparency pass; pipeline state not found\n";
	}
}

// Primary rendering method; executes all deferred rendering operations
void DeferredRenderProcess::Render(void)
{
	/*
		1. Populate common constant buffers
		2. Clear GBuffer render target
		3. Render all opaque geometry
		4. Copy GBuffer depth/stencil to primary render target
		5a. Lighting pass 1: determine lit pixels (non-directional lights)
		5b. Lighting pass 2: render lit pixels (non-directional lights)
		5c: Lighting: render directional lights
		6. Render transparent objects
	*/

	/* 1. Populate common constant buffers */
	PopulateCommonConstantBuffers();

	/* 2. Clear GBuffer RT */
	GBuffer.RenderTarget->Clear(ClearFlags::All, NULL_FLOAT4, 1.0f, 0U);

	/* 3. Render opaque geometry */
	RenderGeometry();

	/* 4. Copy GBuffer depth/stencil to primary render target */
	Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->
		GetTexture(RenderTarget::AttachmentPoint::DepthStencil)->Copy(
		GBuffer.DepthStencilTexture);

	/* 5. Perform deferred lighting */
	PerformDeferredLighting();

	/* 6. Render transparent objects */
	RenderTransparency();
}


void DeferredRenderProcess::PopulateCommonConstantBuffers(void)
{
	// Frame data buffer
	m_cb_frame_data.RawPtr->View = Game::Engine->GetRenderViewMatrixF();
	m_cb_frame_data.RawPtr->Projection = Game::Engine->GetRenderProjectionMatrixF();
	m_cb_frame_data.RawPtr->InvProjection = Game::Engine->GetRenderInverseProjectionMatrixF();
	m_cb_frame_data.RawPtr->ScreenDimensions = Game::Engine->GetRenderDevice()->GetDisplaySizeF();
	m_cb_frame->Set(m_cb_frame_data.RawPtr);

}


void DeferredRenderProcess::RenderGeometry(void)
{
	// Bind required buffer resources to shader parameters
	m_pipeline_geometry->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_ps_geom_framedata).Set(Game::Engine->GetRenderDevice()->GetCommonFrameDataBuffer());

	// Bind the entire geometry rendering pipeline, including all shaders, render targets & states
	m_pipeline_geometry->Bind();

	// Render all non-transparent objects
	*** DO THIS ***

	// Unbind the geometry rendering pipeline
	// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
	m_pipeline_geometry->Unbind();

	*** TODO: NEED TO CALL CLEARRENDERQUEUE IN THE ENGINE RENDER() METHOD ITSELF AFTER THIS RENDER() METHOD IS CALLED ***
}

void DeferredRenderProcess::PerformDeferredLighting(void)
{

}

void DeferredRenderProcess::RenderTransparency(void)
{

}






DeferredRenderProcess::DeferredRenderProcess(void)
{

}