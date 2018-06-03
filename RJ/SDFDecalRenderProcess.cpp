#include "SDFDecalRenderProcess.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "DecalRenderingMode.h"
#include "DecalRenderingManager.h"
#include "DecalRenderingParams.h"
#include "Data/Shaders/SDFDecalRenderingCommonData.hlsl.h"


// Constructor
SDFDecalRenderProcess::SDFDecalRenderProcess(void)
	:
	m_vs(NULL),
	m_ps_direct(NULL),
	m_ps_deferred(NULL), 
	m_pipeline_direct(NULL),
	m_pipeline_deferredproj(NULL), 
	m_cb_frame(NULL),
	m_cb_frame_mode(FrameBufferMode::Uninitialised), 
	m_model_quad(NULL),
	m_decal_material(NULL), 
	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_direct_decaldata(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_deferred_decaldata(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	SetName(RenderProcess::Name<SDFDecalRenderProcess>());

	InitialiseShaders();
	InitialiseStandardBuffers();
	InitialiseStandardMaterials();
	InitialisePipelines();
	InitialiseShaderResourceBindings();
}

// Perform any initialisation that cannot be completed on construction, e.g. because it requires
// data that is read in from disk during the data load process
void SDFDecalRenderProcess::PerformPostDataLoadInitialisation(void)
{
	Game::Log << LOG_INFO << "Performing post-data load initialisation of SDF decal rendering process\n";

	// Can only be performed once model data is read from external data files
	InitialiseRenderGeometry();
}

// Initialisation
void SDFDecalRenderProcess::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising SDF decal rendering shaders\n";

	// Get a reference to all required shaders
	m_vs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::SDFDecalVertexShader);
	if (m_vs == NULL) Game::Log << LOG_ERROR << "Cannot load SDF decal rendering shader resources [vs]\n";

	m_ps_direct = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::SDFDecalDirectPixelShader);
	if (m_ps_direct == NULL) Game::Log << LOG_ERROR << "Cannot load SDF decal rendering shader resources [ps_dir]\n";

	m_ps_deferred = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::SDFDecalDeferredPixelShader);
	if (m_ps_deferred == NULL) Game::Log << LOG_ERROR << "Cannot load SDF decal rendering shader resources [ps_def]\n";

	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framedata = AttemptRetrievalOfShaderParameter(m_vs, DecalRenderingFrameBufferName);
	m_param_ps_direct_decaldata = AttemptRetrievalOfShaderParameter(m_ps_direct, DecalRenderingDataBufferName);
	m_param_ps_deferred_decaldata = AttemptRetrievalOfShaderParameter(m_ps_deferred, DecalRenderingDataBufferName);
}

// Initialisation
void SDFDecalRenderProcess::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising SDF decal rendering standard buffer resources\n";

	// Frame data buffer
	m_cb_frame = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<DecalRenderingFrameBuffer>(DecalRenderingFrameBufferName, m_cb_frame_data.RawPtr);
	m_cb_decal = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<DecalRenderingDataBuffer>(DecalRenderingDataBufferName, m_cb_decal_data.RawPtr);
}

// Initialisation
void SDFDecalRenderProcess::InitialiseStandardMaterials(void)
{
	Game::Log << LOG_INFO << "Initialising SDF decal rendering standard material resources\n";

	// Will be populated with required decal texture for each pass of the render process
	m_decal_material = ManagedPtr<MaterialDX11>(new MaterialDX11("sdf-decal"));
}

// Initialisation
void SDFDecalRenderProcess::InitialiseRenderGeometry(void)
{
	Game::Log << LOG_INFO << "Initialising SDF decal rendering geometry\n";

	// Load all required model geometry
	std::vector<std::tuple<std::string, std::string, Model**>> models = {
		{ "texture quad", "unit_square_model", &m_model_quad }
	};

	for (auto & model : models)
	{
		Model *m = Model::GetModel(std::get<1>(model));
		if (!m)
		{
			Game::Log << LOG_ERROR << "Could not load " << std::get<0>(model) << " model (\"" << std::get<1>(model) << "\") during SDF decal render process initialisation\n";
		}

		*(std::get<2>(model)) = m;
	}
}

// Initialisation
void SDFDecalRenderProcess::InitialisePipelines(void)
{
	// Direct rendering pipeline
	Game::Log << LOG_INFO << "Initialising SDF decal rendering pipeline [dir]\n";
	m_pipeline_direct= Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("SDF_Decal_Rendering_Direct");
	m_pipeline_direct->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_direct->SetShader(Shader::Type::PixelShader, m_ps_direct);
	m_pipeline_direct->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());

	m_pipeline_direct->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline_direct->GetRasterizerState().SetDepthClipEnabled(false);
	m_pipeline_direct->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(false, DepthStencilState::DepthWrite::Disable));
	m_pipeline_direct->GetBlendState().SetBlendMode(BlendState::BlendModes::AlphaBlend);

	// Deferred screen-space projection rendering pipeline
	Game::Log << LOG_INFO << "Initialising SDF decal rendering pipeline [def]\n";
	m_pipeline_deferredproj = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("SDF_Decal_Rendering_Deferred");
	m_pipeline_deferredproj->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline_deferredproj->SetShader(Shader::Type::PixelShader, m_ps_deferred);
	m_pipeline_deferredproj->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());

	m_pipeline_deferredproj->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline_deferredproj->GetRasterizerState().SetDepthClipEnabled(false);
	m_pipeline_deferredproj->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(true, DepthStencilState::DepthWrite::Disable));
	m_pipeline_deferredproj->GetBlendState().SetBlendMode(BlendState::BlendModes::AlphaBlend);
}

// Bind required buffer resources to shader parameters
void SDFDecalRenderProcess::InitialiseShaderResourceBindings(void)
{
	// Vertex shader
	m_vs->GetParameter(m_param_vs_framedata).Set(GetFrameDataBuffer());

	// Pixel shaders
	m_ps_direct->GetParameter(m_param_ps_direct_decaldata).Set(GetDecalRenderingConstantBuffer());
	m_ps_deferred->GetParameter(m_param_ps_deferred_decaldata).Set(GetDecalRenderingConstantBuffer());
}



// Virtual render method; must be implemented by all derived render processess
void SDFDecalRenderProcess::Render(void)
{
	// Per-frame initialisation
	m_cb_frame_mode = FrameBufferMode::Uninitialised;

	// Process each queued render group in turn
	const auto & render_queue = Game::Engine->GetDecalRenderer()->GetQueuedRenderingData();
	for (const auto & group : render_queue)
	{
		// Only process valid & non-empty groups
		if (!group.IsInUse()) continue;

		// Populate the shader CB with data for this render group
		PopulateDecalDataBuffer(group);

		// Update the decal rendering material with the correct texture
		m_decal_material.RawPtr->SetTexture(Material::TextureType::Diffuse, group.GetTexture());

		// Render via the appropriate pipeline
		switch (group.GetRenderingMode())
		{
			case DecalRenderingMode::ScreenSpace:
				PopulateFrameDataBuffer(FrameBufferMode::ScreenSpace);
				ExecuteRenderingPipeline(m_pipeline_direct, group);
				break;
			case DecalRenderingMode::WorldSpace:
				PopulateFrameDataBuffer(FrameBufferMode::WorldSpace);
				ExecuteRenderingPipeline(m_pipeline_direct, group);
				break;
			case DecalRenderingMode::DeferredWorldProjection:
				PopulateFrameDataBuffer(FrameBufferMode::WorldSpace);
				ExecuteRenderingPipeline(m_pipeline_deferredproj, group);
				break;
		}
		
	}
}

void SDFDecalRenderProcess::ExecuteRenderingPipeline(PipelineStateDX11 * pipeline, const DecalRenderingParams & render_group)
{
	// Bind the entire geometry rendering pipeline, including all shaders, render targets & states
	pipeline->Bind();

	// Perform instanced rendering of the full queued render group through this pipeline
	const auto & instances = render_group.GetQueuedInstanceData();
	Game::Engine->RenderInstanced(*pipeline, *m_model_quad, m_decal_material.RawPtr, instances[0], static_cast<UINT>(instances.size()));

	// Unbind the geometry rendering pipeline
	// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
	pipeline->Unbind();
}


// Populate frame data buffer for the given rendering mode
void SDFDecalRenderProcess::PopulateFrameDataBuffer(SDFDecalRenderProcess::FrameBufferMode frame_buffer_mode)
{
	// Populate based on the desired rendering mode
	if (frame_buffer_mode == FrameBufferMode::ScreenSpace)
	{
		if (m_cb_frame_mode == FrameBufferMode::ScreenSpace) return;		// Already populated
		m_cb_frame_mode = FrameBufferMode::ScreenSpace;

		m_cb_frame_data.RawPtr->ViewProjection = Game::Engine->GetRenderOrthographicMatrixF();		// (View = ID, Proj = Ortho)
		m_cb_frame->Set(m_cb_frame_data.RawPtr);
	}
	else if (frame_buffer_mode == FrameBufferMode::WorldSpace)
	{
		if (m_cb_frame_mode == FrameBufferMode::WorldSpace) return;			// Already populated
		m_cb_frame_mode = FrameBufferMode::WorldSpace;

		m_cb_frame_data.RawPtr->ViewProjection = ID_MATRIX_F;	// TODO: UPDATE.  MAY NEED TO SPLIT VIEW VS PROJ (OR POPULATE BOTH OPTIONS)
		m_cb_frame->Set(m_cb_frame_data.RawPtr);
	}
}

// Populate standard decal data required for each pass of the decal rendering process
void SDFDecalRenderProcess::PopulateDecalDataBuffer(const DecalRenderingParams & render_group)
{
	// Decal rendering buffer (ps)
	m_cb_decal_data.RawPtr->baseColour = render_group.GetBaseColour();
	m_cb_decal_data.RawPtr->outlineColour = render_group.GetOutlineColour();
	m_cb_decal_data.RawPtr->outlineDistanceFactor = render_group.GetOutlineWidthFactor();
	m_cb_decal_data.RawPtr->smoothingFactor = render_group.GetSmoothingFactor();
	m_cb_decal->Set(m_cb_decal_data.RawPtr);

}

