#include "DeferredRenderProcess.h"
#include "CoreEngine.h"
#include "LightingManagerObject.h"
#include "RenderDeviceDX11.h"
#include "PipelineStateDX11.h"
#include "ShaderDX11.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Model.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "Data/Shaders/LightDataBuffers.hlsl"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"


DeferredRenderProcess::DeferredRenderProcess(void)
	:
	m_vs(NULL), 
	m_ps_geometry(NULL), 
	m_ps_lighting(NULL), 
	m_depth_only_rt(NULL), 
	m_cb_frame(NULL), 
	m_cb_lightindex(NULL), 

	m_pipeline_geometry(NULL), 
	m_pipeline_lighting_pass1(NULL), 
	m_pipeline_lighting_pass2(NULL), 
	m_pipeline_lighting_directional(NULL), 
	m_pipeline_transparency(NULL), 

	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_framedata(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_light_lightdata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_lightindexdata(ShaderDX11::INVALID_SHADER_PARAMETER), 

	m_model_sphere(NULL), 
	m_model_cone(NULL)
{
	InitialiseShaders();
	InitialiseRenderTargets();
	InitialiseStandardBuffers();
	InitialiseRenderVolumes();

	InitialiseGeometryPipelines();
	InitialiseDeferredLightingPipelines();
	InitialiseDeferredDirectionalLightingPipeline();
	InitialiseTransparentRenderingPipelines();
}

void DeferredRenderProcess::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering shaders\n";

	// Get a reference to all required shaders
	m_vs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::StandardVertexShader);
	if (m_vs == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [vs]\n";

	m_ps_geometry = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredGeometryPixelShader);
	if (m_ps_geometry == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_g]\n";

	m_ps_lighting = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredLightingPixelShader);
	if (m_ps_lighting == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_l]\n";

	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framedata = AttemptRetrievalOfShaderParameter(m_vs, FrameDataBufferName);
	m_param_ps_light_framedata = AttemptRetrievalOfShaderParameter(m_ps_lighting, FrameDataBufferName);
	m_param_ps_light_lightdata = AttemptRetrievalOfShaderParameter(m_ps_lighting, LightBufferName);
	m_param_ps_light_lightindexdata = AttemptRetrievalOfShaderParameter(m_ps_lighting, LightIndexBufferName);
}

void DeferredRenderProcess::InitialiseRenderTargets(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering render targets\n";

	// Depth-only render target will be attached to the primary RT depth/stencil buffer
	m_depth_only_rt = Game::Engine->GetRenderDevice()->Assets.CreateRenderTarget("Deferred_DepthOnly", Game::Engine->GetRenderDevice()->GetDisplaySize());
	m_depth_only_rt->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil,
		Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->GetTexture(RenderTarget::AttachmentPoint::DepthStencil));
}

void DeferredRenderProcess::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering standard buffer resources\n";

	m_cb_frame = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<FrameDataBuffer>(FrameDataBufferName, m_cb_frame_data.RawPtr);
	m_cb_lightindex = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<LightIndexBuffer>(LightIndexBufferName, m_cb_lightindex_data.RawPtr);
}

void DeferredRenderProcess::InitialiseRenderVolumes(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering standard render volumes\n";

	std::vector<std::tuple<std::string, std::string, Model**>> models = {
		{ "point light sphere volume", "unit_sphere_model", &m_model_sphere }, 
		{ "spot light cone volume", "unit_cone_model", &m_model_cone}
	};

	for (auto & model : models)
	{
		Model *m = Model::GetModel(std::get<1>(model));
		if (!m)
		{
			Game::Log << LOG_ERROR << "Could not load " << std::get<0>(model) << " model (\"" << std::get<1>(model) << "\") during deferred render process initialisation\n";
		}

		*(std::get<2>(model)) = m;
	}
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
	BeginFrame();

	RenderFrame();

	EndFrame();
}


// Begin the frame; clear per-frame RTs and other resources ready for rendering
void DeferredRenderProcess::BeginFrame(void)
{
	/*
		1. Populate common constant buffers
		2. Clear GBuffer render target
	*/

	/* 1. Populate common constant buffers */
	PopulateCommonConstantBuffers();

	/* 2. Clear GBuffer RT */
	// TODO: REMOVE
	GBuffer.RenderTarget->Clear(ClearFlags::All, /*NULL_FLOAT4*/XMFLOAT4(0.7f,0.7f,0.7f,0.5f), 1.0f, 0U);

}

// Perform all rendering of the frame
void DeferredRenderProcess::RenderFrame(void)
{
	/*
		1. Render all opaque geometry
		2. Copy GBuffer depth/stencil to primary render target
		3a. Lighting pass 1: determine lit pixels (non-directional lights)
		3b. Lighting pass 2: render lit pixels (non-directional lights)
		3c: Lighting: render directional lights
		4. Render transparent objects
	*/
	
	/* 1. Render opaque geometry */
	RenderGeometry();

	/* 2. Copy GBuffer depth/stencil to primary render target */
	Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->
		GetTexture(RenderTarget::AttachmentPoint::DepthStencil)->Copy(
		GBuffer.DepthStencilTexture);

	/* 3. Perform deferred lighting */
	PerformDeferredLighting();

	/* 4. Render transparent objects */
	RenderTransparency();
}

// End the frame, including presentation of swap chain to the primary display
void DeferredRenderProcess::EndFrame(void)
{
	/*
		1. Present backbuffer to the primary display by cycling the swap chain
	*/


	// Present the back buffer to the screen.  Either lock to screen refresh rate (sync 
	// interval = 1, if vsync is enabled) or present as fast as possible (sync 
	// interval = 0, if it is not)
	HRESULT hr = Game::Engine->GetRenderDevice()->PresentFrame();

	// Log presentation failures in debug mode only
#	ifdef _DEBUG
		if (FAILED(hr))
		{
			Game::Log << LOG_ERROR << "Critical: Frame presentation failed with hr=" << hr << "\n";
		}
#	endif
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
	m_pipeline_geometry->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetCommonFrameDataBuffer());

	// Bind the entire geometry rendering pipeline, including all shaders, render targets & states
	m_pipeline_geometry->Bind();

	// Render all non-transparent objects
	Game::Engine->ProcessRenderQueue<ModelRenderPredicate::RenderNonTransparent>(m_pipeline_geometry);

	// Unbind the geometry rendering pipeline
	// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
	m_pipeline_geometry->Unbind();
}

void DeferredRenderProcess::PerformDeferredLighting(void)
{
	// Bind the GBuffer generated in the geometry phase to the deferred lighting pixel shader
	GBuffer.Bind(Shader::Type::PixelShader);

	// Bind required buffer resources to each pipeline
	BindDeferredLightingShaderResources();

	// Process each light in turn
	unsigned int light_count = static_cast<unsigned int>(Game::Engine->LightingManager->GetLightSourceCount());
	const auto * lights = Game::Engine->LightingManager->GetLightData();

	for (unsigned int i = 0U; i < light_count; ++i)
	{
		// Only process active lights
		const auto & light = lights[i];
		if (!light.Enabled) continue;

		// Update the light index buffer
		m_cb_lightindex_data.RawPtr->LightIndex = i;
		m_cb_lightindex->Set(m_cb_lightindex_data.RawPtr);

		// Clear the stencil buffer for rendering of the new light.  Only call for the first
		// pipeline since all three pipelines share the same render target
		m_pipeline_lighting_pass1->GetRenderTarget()->Clear(ClearFlags::Stencil, NULL_FLOAT4, 1.0f, 1);

		// Render based upon light type
		switch (light.Type)
		{
			case LightType::Point:
				RenderLightPipeline(m_pipeline_lighting_pass1, m_model_sphere);
				RenderLightPipeline(m_pipeline_lighting_pass2, m_model_sphere);
				break;

			case LightType::Spotlight:
				RenderLightPipeline(m_pipeline_lighting_pass1, m_model_cone);
				RenderLightPipeline(m_pipeline_lighting_pass2, m_model_cone);

			case LightType::Directional:
				RenderLightPipeline(m_pipeline_lighting_directional, /* TODO: REQUIRED */ NULL);
				break;
		}
	}
	


	// Unbind the GBuffer following all deferred lighting rendering
	GBuffer.Unbind(Shader::Type::PixelShader);
}

// Bind shader resources required for the deferred lighting stage
void DeferredRenderProcess::BindDeferredLightingShaderResources(void)
{
	// TODO: Required for all pipelines when shader/buffer is the same?  E.g. all pipelines use the same frame data param in the same VS
	// TODO: Required every frame?  Only setting buffer pointer in shader.  May only be required on shader reload in case param indices change

	// Lighting pass 1 is VS-only and does not output fragments
	m_pipeline_lighting_pass1->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetCommonFrameDataBuffer());

	// Lighting pass 2 uses both VS and PS
	m_pipeline_lighting_pass2->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_pass2->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_pass2->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightindexdata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_pass2->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightdata).Set(Game::Engine->LightingManager->GetLightDataBuffer());

	// Directional lighting pass uses both VS and PS
	m_pipeline_lighting_directional->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_directional->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_directional->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightindexdata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_directional->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightdata).Set(Game::Engine->LightingManager->GetLightDataBuffer());

}

// Render a subset of the deferred lighting phase using the given pipeline and light render volume
void DeferredRenderProcess::RenderLightPipeline(PipelineStateDX11 *pipeline, Model *light_render_volume)
{
	pipeline->Bind();

	/***
		Generate instance data for the model based on light range/position/etc. as per INFOSPEC::DeferredLightingPass::Visit
		We can now render using "CoreEngine::RenderInstanced(const PipelineStateDX11 & pipeline, const ModelBuffer & model, const RM_Instance & instance_data, UINT instance_count)"
	***/

	pipeline->Unbind();
}

void DeferredRenderProcess::RenderTransparency(void)
{

}






DeferredRenderProcess::~DeferredRenderProcess(void)
{

}