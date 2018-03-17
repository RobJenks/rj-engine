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
	m_ps_debug(NULL), 
	m_depth_only_rt(NULL), 
	m_cb_frame(NULL), 
	m_cb_lightindex(NULL), 
	m_cb_debug(NULL), 

	m_pipeline_geometry(NULL), 
	m_pipeline_lighting_pass1(NULL), 
	m_pipeline_lighting_pass2(NULL), 
	m_pipeline_lighting_directional(NULL), 
	m_pipeline_transparency(NULL), 
	m_pipeline_debug_rendering(NULL), 

	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_framedata(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_light_lightdata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_lightindexdata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_debug_debugdata(ShaderDX11::INVALID_SHADER_PARAMETER), 

	m_model_sphere(NULL), 
	m_model_cone(NULL),
	m_model_quad(NULL), 
	m_transform_fullscreen_quad(ID_MATRIX), 

	m_debug_render_mode(DeferredRenderProcess::DebugRenderMode::None)
{
	m_name = RenderProcess::Name<DeferredRenderProcess>();

	InitialiseShaders();
	InitialiseRenderTargets();
	InitialiseStandardBuffers();

	InitialiseGeometryPipelines();
	InitialiseDeferredLightingPipelines();
	InitialiseDeferredDirectionalLightingPipeline();
	InitialiseTransparentRenderingPipelines();
	InitialiseDebugRenderingPipelines();
}

// Perform any initialisation that cannot be completed on construction, e.g. because it requires
// data that is read in from disk during the data load process
void DeferredRenderProcess::PerformPostDataLoadInitialisation(void)
{
	Game::Log << LOG_INFO << "Performing post-data load initialisation of deferred render process\n";

	// Can only be performed once model data is read from external data files
	InitialiseRenderVolumes();
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

#ifdef _DEBUG
	m_ps_debug = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredLightingDebug);
	if (m_ps_debug == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering debug shader resources [ps_d]\n";
	m_param_ps_debug_debugdata = AttemptRetrievalOfShaderParameter(m_ps_debug, DeferredDebugBufferName);
#endif

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

#ifdef _DEBUG
	m_cb_debug = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<DeferredDebugBuffer>(DeferredDebugBufferName, m_cb_debug_data.RawPtr);
#endif
}

void DeferredRenderProcess::InitialiseRenderVolumes(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering standard render volumes\n";

	// Load all required model geometry
	std::vector<std::tuple<std::string, std::string, Model**>> models = {
		{ "point light sphere volume", "unit_sphere_model", &m_model_sphere }, 
		{ "spot light cone volume", "spotlight_cone_model", &m_model_cone}, 
		{ "fullscreen quad", "unit_square_model", &m_model_quad }
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

	// Also precalculate a fullscreen quad transform for rendering directional lights via the quad model
	auto displaysize = Game::Engine->GetRenderDevice()->GetDisplaySizeF();
	m_transform_fullscreen_quad = XMMatrixMultiply(
		XMMatrixScaling(displaysize.x, displaysize.y, 1.0f), 
		XMMatrixTranslation(0.0f, 0.0f, 10.0f)
	);
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
	//m_pipeline_lighting_pass2->GetBlendState().SetBlendMode(BlendState::BlendMode(true, false, BlendState::BlendFactor::One, BlendState::BlendFactor::Zero)); // TODO: REMOVE (DEBUG TO RENDER *ONLY* OUTPUT OF PASS2)

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

void DeferredRenderProcess::InitialiseDebugRenderingPipelines(void)
{
	// Debug pipeline is only active in debug builds
#ifndef _DEBUG
	Game::Log << LOG_INFO << "Skipping initialisation of deferred rendering pipeline [d]; not required\n";
	return;
#endif

	// Initialise pipeline
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [d]\n";

	m_pipeline_debug_rendering = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Lighting_Debug");
	m_pipeline_debug_rendering->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_debug_rendering->SetShader(Shader::Type::PixelShader, m_ps_debug);
	m_pipeline_debug_rendering->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(false));		// Disable all depth testing
	m_pipeline_debug_rendering->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);				// No culling for fullscreen quad rendering
	m_pipeline_debug_rendering->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());
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
		5. [Debug only] If debug GBuffer rendering is enabled, overwrite all primary RT data with the debug output
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

	/* 5. If debug rendering from the GBuffer is enabled, run the debug pipeline and overwrite all primary RT data */
#ifdef _DEBUG
	GBufferDebugRendering();
#endif
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

void DeferredRenderProcess::PopulateFrameBufferForFullscreenQuadRendering(void)
{
	// Frame data buffer
	m_cb_frame_data.RawPtr->View = ID_MATRIX_F;														// View matrix == identity
	m_cb_frame_data.RawPtr->Projection = Game::Engine->GetRenderOrthographicMatrixF();				// Proj matrix == orthographic
	m_cb_frame_data.RawPtr->InvProjection = Game::Engine->GetRenderInverseOrthographicMatrixF();	// Inv proj == inv orthographic
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
	XMMATRIX transform;

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
				transform = LightSource::CalculatePointLightTransform(light);
				RenderLightPipeline(m_pipeline_lighting_pass1, m_model_sphere, transform);
				RenderLightPipeline(m_pipeline_lighting_pass2, m_model_sphere, transform);
				break;

			case LightType::Spotlight:
				transform = LightSource::CalculateSpotlightTransform(light);
				RenderLightPipeline(m_pipeline_lighting_pass1, m_model_cone, transform);
				RenderLightPipeline(m_pipeline_lighting_pass2, m_model_cone, transform);
				break;

			case LightType::Directional:
				//RenderLightPipeline(m_pipeline_lighting_directional, /* TODO: REQUIRED */ NULL, ID_MATRIX);
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
	m_pipeline_lighting_pass2->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightindexdata).Set(m_cb_lightindex);
	m_pipeline_lighting_pass2->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightdata).Set(Game::Engine->LightingManager->GetLightDataBuffer());

	// Directional lighting pass uses both VS and PS
	m_pipeline_lighting_directional->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_directional->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_lighting_directional->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightindexdata).Set(m_cb_lightindex);
	m_pipeline_lighting_directional->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_light_lightdata).Set(Game::Engine->LightingManager->GetLightDataBuffer());

}

// Render a subset of the deferred lighting phase using the given pipeline and light render volume
void DeferredRenderProcess::RenderLightPipeline(PipelineStateDX11 *pipeline, Model *light_render_volume, const FXMMATRIX transform)
{
	// Simply render a single instance of the light volume within the bound pipeline
	pipeline->Bind();
	Game::Engine->RenderInstanced(*pipeline, light_render_volume->Data, RM_Instance(transform), 1U);
	pipeline->Unbind();
}

void DeferredRenderProcess::RenderTransparency(void)
{

}



// Redirect an alternative render output to the primary render target Color0, and ultimately the backbuffer
bool DeferredRenderProcess::RepointBackbufferRenderTargetAttachment(const std::string & target)
{
	auto type = StrLower(target);

	if (type == "diffuse")				m_debug_render_mode = DebugRenderMode::Diffuse;
	else if (type == "specular")		m_debug_render_mode = DebugRenderMode::Specular;
	else if (type == "normal")			m_debug_render_mode = DebugRenderMode::Normal;
	else if (type == "depth")			m_debug_render_mode = DebugRenderMode::Depth;
	else
	{
		// Unrecognised mode
		m_debug_render_mode = DebugRenderMode::None;
		return false;
	}

	// If we reach this point a debug mode WAS set, so return success
	return true;
}

TextureDX11 * DeferredRenderProcess::GetDebugTexture(DeferredRenderProcess::DebugRenderMode debug_mode)
{
	switch (debug_mode)
	{
		// GBuffer textures
		case DebugRenderMode::Diffuse:		return GBuffer.DiffuseTexture;
		case DebugRenderMode::Specular:		return GBuffer.SpecularTexture;
		case DebugRenderMode::Normal:		return GBuffer.NormalTexture;
		case DebugRenderMode::Depth:		return GBuffer.DepthStencilTexture;

		// Other textures


		// Unknown texture
		default:							return NULL;
	}
}

// Perform debug rendering of GBuffer data, if enabled.  Returns a flag indicating whether debug rendering was performed
bool DeferredRenderProcess::GBufferDebugRendering(void)
{
	// Normal case: exit immediately since debug rendering is not enabled
	if (m_debug_render_mode == DebugRenderMode::None) return false;

	// Bind the texture to be debug-rendered at register t0
	TextureDX11 * texture = GetDebugTexture(m_debug_render_mode);
	if (!texture) return false;
	texture->Bind(Shader::Type::PixelShader, 0U, ShaderParameter::Type::Texture);

	// Populate frame data buffer for fullscreen rendering
	PopulateFrameBufferForFullscreenQuadRendering();

	// Populate debug rendering constant buffer
	m_cb_debug_data.RawPtr->is_depth_texture = (m_debug_render_mode == DebugRenderMode::Depth ? TRUE : FALSE);
	m_cb_debug->Set(m_cb_debug_data.RawPtr);
	
	// Bind shader parameters to the debug pipeline
	m_pipeline_debug_rendering->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetCommonFrameDataBuffer());
	m_pipeline_debug_rendering->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_debug_debugdata).Set(m_cb_debug);

	// Bind the debug pipeline
	m_pipeline_debug_rendering->Bind();

	// Render a full-screen quad through the debug pipeline.  Debug texture will be rendered directly to this quad
	Game::Engine->RenderInstanced(*m_pipeline_debug_rendering, m_model_quad->Data, RM_Instance(m_transform_fullscreen_quad), 1U);

	// Repopulate the frame data buffer with normal data
	PopulateCommonConstantBuffers();

	// Unbind the debug pipeline following rendering
	m_pipeline_debug_rendering->Unbind();

	return true;
}


DeferredRenderProcess::~DeferredRenderProcess(void)
{

}