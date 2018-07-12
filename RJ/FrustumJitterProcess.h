#pragma once

#include <vector>
#include "DX11_Core.h"
#include "CompilerSettings.h"


class FrustumJitterProcess
{
public:

	// Constructor
	FrustumJitterProcess(void);

	// Indicates whether frustum jittering is currently enabled
	CMPINLINE bool										IsEnabled(void) const { return m_enabled; }

	// Calculate frustum jitter for the frame, if enabled
	void												Update(void);

	// Apply frustum jittering to the given [projection] matrix and return the result
	XMMATRIX											Apply(FXMMATRIX matrix);

	// Enable or disable the process
	CMPINLINE void										SetEnabled(bool enabled)	{ m_enabled = enabled; }
	CMPINLINE void										Enable(void)				{ SetEnabled(true); }
	CMPINLINE void										Disable(void)				{ SetEnabled(false); }

	// Scaling factor applied to the jitter process
	CMPINLINE float										GetJitterScale(void) const	{ return m_jitter_scale; }
	CMPINLINE void										SetJitterScale(float scale) { m_jitter_scale = scale; }

	// Return the jitter translation for the current frame
	CMPINLINE XMVECTOR									GetJitterVector(void) const { return m_frame_jitter; }
	CMPINLINE XMFLOAT2									GetJitterVectorF(void) const { return m_frame_jitter_f; }

	// Return the jitter translation matrix for the current frame
	CMPINLINE XMMATRIX									GetJitterMatrix(void) const { return m_frame_jitter_matrix; }

	// Destructor
	~FrustumJitterProcess(void);

private:

	bool												m_enabled;
	float												m_jitter_scale;

	XMVECTOR											m_frame_jitter;
	XMFLOAT2											m_frame_jitter_f;
	XMMATRIX											m_frame_jitter_matrix;

	// Using Halton-2-3 (16) distribution for jitter process
	static const unsigned int 							JITTER_DISTRIBUTION_SIZE = 16U;
	std::vector<XMFLOAT2>								m_jitter_distribution;

};