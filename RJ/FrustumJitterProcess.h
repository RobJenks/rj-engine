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
	void												Update(XMFLOAT2 displaysize);

	// Apply frustum jittering to the given [projection] matrix and return the result
	XMMATRIX											Apply(FXMMATRIX matrix);

	// Enable or disable the process
	void												SetEnabled(bool enabled);
	CMPINLINE void										Enable(void)				{ SetEnabled(true); }
	CMPINLINE void										Disable(void)				{ SetEnabled(false); }

	// Reset the component state ready to begin from the first frame
	void												Reset(void);

	// Scaling factor applied to the jitter process
	CMPINLINE float										GetJitterScale(void) const	{ return m_jitter_scale; }
	CMPINLINE void										SetJitterScale(float scale) { m_jitter_scale = scale; }

	// Return the jitter translation for the current frame
	CMPINLINE XMVECTOR									GetJitterVector(void) const { return m_frame_jitter; }
	CMPINLINE XMFLOAT2									GetJitterVectorF(void) const { return m_frame_jitter_f; }

	// Return the jitter translation matrix for the current frame
	CMPINLINE XMMATRIX									GetJitterMatrix(void) const { return m_frame_jitter_matrix; }

	// Return the two-frame jitter values.  Current frame jitter in xy, previous frame in zw
	CMPINLINE XMFLOAT4									GetTwoFrameJitterVectorF(void) const { return m_two_frame_jitter; }

	// Destructor
	~FrustumJitterProcess(void);

private:

	bool												m_enabled;
	float												m_jitter_scale;

	XMVECTOR											m_frame_jitter;
	XMFLOAT2											m_frame_jitter_f;
	XMMATRIX											m_frame_jitter_matrix;

	// Current frame jitter in xy, previous frame in zw
	XMFLOAT4											m_two_frame_jitter;

	// Using Halton-2-3 (16) distribution for jitter process
	static const unsigned int 							JITTER_DISTRIBUTION_SIZE = 16U;
	std::vector<XMFLOAT2>								m_jitter_distribution;

};