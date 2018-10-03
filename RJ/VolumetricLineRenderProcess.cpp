#include "VolumetricLineRenderProcess.h"

#include "Logging.h"
#include "CoreEngine.h"
#include "DeferredGBuffer.h"
#include "RenderDeviceDX11.h"
#include "RenderAssetsDX11.h"


// Constructor
VolumetricLineRenderProcess::VolumetricLineRenderProcess(void)
	:
	m_vs(NULL), 
	m_gs(NULL), 
	m_ps(NULL), 

	m_cb_framebuffer(NULL), 
	m_pipeline(NULL), 

	m_param_vs_framebuffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_gs_framebuffer(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_framebuffer(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_depth_texture(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	SetName(RenderProcess::Name<VolumetricLineRenderProcess>());

	InitialiseShaders();
	InitialiseStandardBuffers();
	InitialisePipelines();
	InitialiseShaderResourceBindings();
}

// Perform any initialisation that cannot be completed on construction, e.g. because it requires
// data that is read in from disk during the data load process
void VolumetricLineRenderProcess::PerformPostDataLoadInitialisation(void)
{
}

// Initialisation method
void VolumetricLineRenderProcess::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising volumetric line rendering shaders\n";

	// Get a reference to all required shaders
	m_vs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::VolumetricLineVertexShader);
	if (m_vs == NULL) Game::Log << LOG_ERROR << "Cannot load volumetric line rendering shader resources [vs]\n";

	m_gs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::VolumetricLineGeometryShader);
	if (m_gs == NULL) Game::Log << LOG_ERROR << "Cannot load volumetric line rendering shader resources [gs]\n";

	m_ps = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::VolumetricLinePixelShader);
	if (m_ps == NULL) Game::Log << LOG_ERROR << "Cannot load volumetric line rendering shader resources [ps]\n";

	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framebuffer = AttemptRetrievalOfShaderParameter(m_vs, VolumetricLineRenderingFrameBufferName);
	m_param_gs_framebuffer = AttemptRetrievalOfShaderParameter(m_gs, VolumetricLineRenderingFrameBufferName);
	m_param_ps_framebuffer = AttemptRetrievalOfShaderParameter(m_ps, VolumetricLineRenderingFrameBufferName);
	m_param_ps_depth_texture = AttemptRetrievalOfShaderParameter(m_ps, VLDepthBufferInputName);
}

// Respond to a change in shader configuration or a reload of shader bytecode
void VolumetricLineRenderProcess::ShadersReloaded(void)
{
	Game::Log << LOG_INFO << "Reinitialising volumetric line render processes following shader reload\n";

	InitialiseShaders();
}

// Initialisation method
void VolumetricLineRenderProcess::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising volumetric line rendering buffers\n";

	m_cb_framebuffer = Game::Engine->GetAssets().CreateConstantBuffer(VolumetricLineRenderingFrameBufferName, m_cb_framebuffer_data.RawPtr);
}

// Initialisation method
void VolumetricLineRenderProcess::InitialisePipelines(void)
{
	Game::Log << LOG_INFO << "Initialising volumetric line rendering pipeline\n";

	m_pipeline = Game::Engine->GetAssets().CreatePipelineState("VolumetricLine");
	
	m_pipeline->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline->SetShader(Shader::Type::GeometryShader, m_gs);
	m_pipeline->SetShader(Shader::Type::PixelShader, m_ps);

	// TODO: verify against previous configuration
	m_pipeline->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline->GetBlendState().SetBlendMode(BlendState::BlendModes::AdditiveBlend);
	m_pipeline->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(false, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Always));
	
	// Set default render target of the primary colour buffer
	SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());
}

// Initialisation method
void VolumetricLineRenderProcess::InitialiseShaderResourceBindings(void)
{
	m_vs->SetParameterData(m_param_vs_framebuffer, m_cb_framebuffer);
	m_gs->SetParameterData(m_param_gs_framebuffer, m_cb_framebuffer);
	m_ps->SetParameterData(m_param_ps_framebuffer, m_cb_framebuffer);
}

// Allow redirect of render process output to an alternative render target.  Default is the 
// primary colour buffer
void VolumetricLineRenderProcess::SetRenderTarget(RenderTargetDX11 *rendertarget)
{
	m_pipeline->SetRenderTarget(rendertarget);
}

void VolumetricLineRenderProcess::PopulateFrameBuffer(void)
{
	m_cb_framebuffer_data.RawPtr->C_ViewMatrix = Game::Engine->GetRenderViewMatrixF();
	m_cb_framebuffer_data.RawPtr->C_ProjMatrix = Game::Engine->GetRenderViewProjectionMatrixF();
	m_cb_framebuffer_data.RawPtr->C_ViewportSize = Game::Engine->GetRenderDevice()->GetDisplaySizeF();
	m_cb_framebuffer_data.RawPtr->C_NearClipDistance = Game::Engine->GetRenderDevice()->GetNearClipDistance();
	m_cb_framebuffer_data.RawPtr->C_FarClipDistance = Game::Engine->GetRenderDevice()->GetFarClipDistance();

	m_cb_framebuffer->Set(m_cb_framebuffer_data.RawPtr);
}

// Virtual render method; must be implemented by all derived render processess
void VolumetricLineRenderProcess::Render(void)
{
	// This render pass will use GBuffer data from the prior deferred rendering pass
	auto gbuffer = Game::Engine->GetGBufferReference();
	assert(gbuffer != NULL); 
	if (!gbuffer) return;

	// Populate common buffer data
	PopulateFrameBuffer();

	// Bind required per-frame resources to shader parameters
	// TODO: Can use GBuffer depth texture?  Need to use the primary DSV to ensure everything is included?  If so, could be a 
	// problem since the primary DSV will currently be bound to a render target output and cannot be used as an input
	m_ps->SetParameterData(m_param_ps_depth_texture, gbuffer->DepthStencilTexture);

	// Bind the entire geometry rendering pipeline, including all shaders, render targets & states
	m_pipeline->Bind();

	// Render all non-transparent objects
	Game::Engine->ProcessRenderQueue<ShaderRenderPredicate::RenderVolumetricLine, ModelRenderPredicate::RenderAll>(m_pipeline);

	// Unbind the geometry rendering pipeline
	// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
	m_pipeline->Unbind();
}


