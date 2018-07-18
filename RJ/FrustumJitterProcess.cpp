#include "FrustumJitterProcess.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "JitterSampleDistribution.h"


// Constructor
FrustumJitterProcess::FrustumJitterProcess(void)
	:
	m_enabled(false),
	m_jitter_scale(1.0f), 
	m_frame_jitter(NULL_VECTOR), 
	m_frame_jitter_f(NULL_FLOAT2), 
	m_frame_jitter_matrix(ID_MATRIX), 
	m_two_frame_jitter(NULL_FLOAT4)
{
	// Initialise the desired jitter sample distribution; currently a Halton-2-3 of size 16
	// TODO: Allow other distributions for comparison?
	JitterSampleDistribution::GenerateHalton23Distribution(m_jitter_distribution, JITTER_DISTRIBUTION_SIZE);

	// Reset the component state ready to begin from the first frame
	Reset();
}

// Enable or disable the process
void FrustumJitterProcess::SetEnabled(bool enabled) 
{ 
	m_enabled = enabled; 
	Reset();
}

// Reset the component state ready to begin from the first frame
void FrustumJitterProcess::Reset(void)
{
	m_frame_jitter_f = NULL_FLOAT2;
	m_frame_jitter = NULL_VECTOR;
	m_frame_jitter_matrix = ID_MATRIX;
	m_two_frame_jitter = NULL_FLOAT4;
}

// Calculate a jitter value for the current frame, if enabled
void FrustumJitterProcess::Update(XMFLOAT2 displaysize)
{
	if (!IsEnabled()) return;

	XMFLOAT2 sample = m_jitter_distribution[Game::FrameIndex % JITTER_DISTRIBUTION_SIZE];
	m_frame_jitter_f = XMFLOAT2((sample.x * m_jitter_scale) / displaysize.x,
								(sample.y * m_jitter_scale) / displaysize.y);

	m_frame_jitter = XMLoadFloat2(&m_frame_jitter_f);
	m_frame_jitter_matrix = XMMatrixTranslationFromVector(m_frame_jitter);

	// Current frame in xy, previous frame in zw
	m_two_frame_jitter = XMFLOAT4(m_two_frame_jitter.z, m_two_frame_jitter.w, m_frame_jitter_f.x, m_frame_jitter_f.y);
}

// Apply frustum jittering to the given [projection] matrix and return the result
XMMATRIX FrustumJitterProcess::Apply(FXMMATRIX matrix)
{
	return XMMatrixMultiply(m_frame_jitter_matrix, matrix);
}

// Destructor
FrustumJitterProcess::~FrustumJitterProcess(void)
{
}


