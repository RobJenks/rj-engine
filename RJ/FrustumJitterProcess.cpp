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
	m_frame_jitter_matrix(ID_MATRIX)
{
	// Initialise the desired jitter sample distribution; currently a Halton-2-3 of size 16
	// TODO: Allow other distributions for comparison?
	JitterSampleDistribution::GenerateHalton23Distribution(m_jitter_distribution, JITTER_DISTRIBUTION_SIZE);
}

// Calculate frustum jitter for the frame, if enabled
void FrustumJitterProcess::Update(void)
{
	if (!IsEnabled()) return;

	m_frame_jitter_f = m_jitter_distribution[Game::FrameIndex % JITTER_DISTRIBUTION_SIZE];
	m_frame_jitter_f.x *= m_jitter_scale;
	m_frame_jitter_f.y *= m_jitter_scale;

	m_frame_jitter = XMLoadFloat2(&m_frame_jitter_f);
	m_frame_jitter_matrix = XMMatrixTranslationFromVector(m_frame_jitter);
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


