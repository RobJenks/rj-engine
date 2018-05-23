#include "SDFDecalRenderProcess.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "DecalRenderingManager.h"
#include "DecalRenderingParams.h"
#include "Data/Shaders/SDFDecalRenderingCommonData.hlsl.h"


// Constructor
SDFDecalRenderProcess::SDFDecalRenderProcess(void)
	:
	m_vs(NULL),
	m_ps(NULL),
	m_pipeline(NULL),
	m_cb_frame(NULL),
	m_model_quad(NULL),
	m_decal_material(NULL), 
	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	SetName(RenderProcess::Name<SDFDecalRenderProcess>());

	InitialiseShaders();
	InitialiseStandardBuffers();
	InitialiseStandardMaterials();
	InitialisePipelines();
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

	m_ps = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::SDFDecalPixelShader);
	if (m_ps == NULL) Game::Log << LOG_ERROR << "Cannot load SDF decal rendering shader resources [ps]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framedata = AttemptRetrievalOfShaderParameter(m_vs, DecalRenderingFrameBufferName);
}

// Initialisation
void SDFDecalRenderProcess::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising SDF decal rendering standard buffer resources\n";

	// Frame data buffer
	m_cb_frame = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<DecalRenderingFrameBuffer>(DecalRenderingFrameBufferName, m_cb_frame_data.RawPtr);
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
	Game::Log << LOG_INFO << "Initialising SDF decal rendering pipeline [*]\n";

	m_pipeline = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("SDF_Decal_Rendering");
	m_pipeline->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline->SetShader(Shader::Type::PixelShader, m_ps);
	m_pipeline->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());

	m_pipeline->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline->GetRasterizerState().SetDepthClipEnabled(false);
	m_pipeline->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(false, DepthStencilState::DepthWrite::Disable));
	m_pipeline->GetBlendState().SetBlendMode(BlendState::BlendModes::AlphaBlend);
}




// Virtual render method; must be implemented by all derived render processess
void SDFDecalRenderProcess::Render(void)
{
	// Process each queued render group in turn
	const auto & render_queue = Game::Engine->GetDecalRenderer()->GetQueuedRenderingData();
	for (const auto & group : render_queue)
	{
		// Only process valid & non-empty groups
		if (!group.IsInUse()) continue;

		// Populate the shader CB with data for this render group
		PopulateFrameBuffer(group);

		// Update the decal rendering material with the correct texture
		m_decal_material.RawPtr->SetTexture(Material::TextureType::Diffuse, group.GetTexture());

		// Bind required buffer resources to shader parameters
		m_pipeline->GetShader(Shader::Type::VertexShader)->GetParameter(m_param_vs_framedata).Set(GetFrameDataBuffer());

		// Bind the entire geometry rendering pipeline, including all shaders, render targets & states
		m_pipeline->Bind();

		// Perform instanced rendering of the full queued render group through this pipeline
		const auto & instances = group.GetQueuedInstanceData();
		Game::Engine->RenderInstanced(*m_pipeline, *m_model_quad, m_decal_material.RawPtr, instances[0], instances.size());

		// Unbind the geometry rendering pipeline
		// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
		m_pipeline->Unbind();
		
	}
}


// Populate standard data required for each pass of the decal rendering process
void SDFDecalRenderProcess::PopulateFrameBuffer(const DecalRenderingParams & render_group)
{
	// Frame data buffer
	// TODO: To be updated
	m_cb_frame_data.RawPtr->ViewProjection = Game::Engine->GetRenderOrthographicMatrixF();		// (View = ID, Proj = Ortho)
	m_cb_frame->Set(m_cb_frame_data.RawPtr);
}