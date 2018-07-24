#include "UIRenderProcess.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"


// Constructor
UIRenderProcess::UIRenderProcess(void)
	:
	m_vs(NULL), 
	m_ps(NULL), 
	m_pipeline(NULL), 
	m_cb_frame(NULL), 
	m_model_quad(NULL), 
	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	SetName( RenderProcess::Name<UIRenderProcess>() );

	InitialiseShaders();
	InitialiseStandardBuffers();
	InitialisePipelines();
}

// Perform any initialisation that cannot be completed on construction, e.g. because it requires
// data that is read in from disk during the data load process
void UIRenderProcess::PerformPostDataLoadInitialisation(void)
{
	Game::Log << LOG_INFO << "Performing post-data load initialisation of UI orthographic render process\n";

	// Can only be performed once model data is read from external data files
	InitialiseRenderGeometry();
}

void UIRenderProcess::InitialiseShaders()
{
	Game::Log << LOG_INFO << "Initialising UI rendering shaders\n";

	// Get a reference to all required shaders
	m_vs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::BasicTextureVertexShader);
	if (m_vs == NULL) Game::Log << LOG_ERROR << "Cannot load UI rendering shader resources [vs]\n";

	m_ps = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::BasicTexturePixelShader);
	if (m_ps == NULL) Game::Log << LOG_ERROR << "Cannot load UI rendering shader resources [ps_g]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framedata = AttemptRetrievalOfShaderParameter(m_vs, BasicTextureRenderingFrameBufferName);

}

void UIRenderProcess::InitialiseStandardBuffers()
{
	Game::Log << LOG_INFO << "Initialising UI rendering standard buffer resources\n";

	// Frame data buffer
	m_cb_frame = Game::Engine->GetRenderDevice()->Assets.CreateConstantBuffer<BasicTextureRenderingFrameBuffer>(BasicTextureRenderingFrameBufferName, m_cb_frame_data.RawPtr);

}

void UIRenderProcess::InitialiseRenderGeometry()
{
	Game::Log << LOG_INFO << "Initialising UI rendering geometry\n";

	// Load all required model geometry
	std::vector<std::tuple<std::string, std::string, Model**>> models = {
		{ "texture quad", "unit_square_model", &m_model_quad }
	};

	for (auto & model : models)
	{
		Model *m = Model::GetModel(std::get<1>(model));
		if (!m)
		{
			Game::Log << LOG_ERROR << "Could not load " << std::get<0>(model) << " model (\"" << std::get<1>(model) << "\") during UI render process initialisation\n";
		}

		*(std::get<2>(model)) = m;
	}
}

void UIRenderProcess::InitialisePipelines()
{
	Game::Log << LOG_INFO << "Initialising UI rendering pipeline [*]\n";

	m_pipeline = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("UI_Rendering");
	m_pipeline->SetShader(Shader::Type::VertexShader, m_vs);
	m_pipeline->SetShader(Shader::Type::PixelShader, m_ps);
	m_pipeline->SetRenderTarget(Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget());

	m_pipeline->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline->GetRasterizerState().SetDepthClipEnabled(false);
	m_pipeline->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(false, DepthStencilState::DepthWrite::Disable));
	m_pipeline->GetBlendState().SetBlendMode(BlendState::BlendModes::AlphaBlend);
}

// Virtual render method; must be implemented by all derived render processess
void UIRenderProcess::Render(void)
{
	// Populate common buffer data
	PopulateFrameBuffer();

	// Bind required buffer resources to shader parameters
	// TODO: Likely not required on each render; only on initialisation
	m_pipeline->GetShader(Shader::Type::VertexShader)->SetParameterData(m_param_vs_framedata, GetFrameDataBuffer());

	// Bind the UI rendering pipeline, including all shaders, render targets & states
	m_pipeline->Bind();

	// Process all UI elements in the render queue
	Game::Engine->ProcessRenderQueue<ShaderRenderPredicate::RenderUI, ModelRenderPredicate::RenderAll>(m_pipeline);

	// Unbind the geometry rendering pipeline
	// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
	m_pipeline->Unbind();
}

void UIRenderProcess::PopulateFrameBuffer(void)
{
	// Frame data buffer
	m_cb_frame_data.RawPtr->ViewProjectionMatrix = Game::Engine->GetRenderOrthographicMatrixF();		// (View = ID, Proj = Ortho)
	m_cb_frame->Set(m_cb_frame_data.RawPtr);
}
