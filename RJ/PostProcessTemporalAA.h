#pragma once

#include <vector>
#include "DX11_Core.h"
#include "PostProcessComponent.h"
#include "ShaderDX11.h"
class DeferredRenderProcess;
class TextureDX11;
class RenderTargetDX11;


class PostProcessTemporalAA : public PostProcessComponent
{
public:

	// Initialise the postprocess and all required resources
	PostProcessTemporalAA(void);
	PostProcessTemporalAA(DeferredRenderProcess * render_process);

	// Reinitialise based on a change to the effect configuration
	void PerformPostConfigInitialisation(void);


	// Destructor
	~PostProcessTemporalAA(void);

private:

	// Initialisation
	void InitialiseShaders(void);
	void InitialiseTextureBuffers(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);



private:

	static const std::string TX_NAME_REPROJECTION0;
	static const std::string TX_NAME_REPROJECTION1;
	static const std::string TX_NAME_FINAL;
	static const std::string RT_NAME_0;
	static const std::string RT_NAME_1;

	ShaderDX11 * m_vs_quad;
	ShaderDX11 * m_ps_temporal;

	TextureDX11 * m_tx_reprojection[2];		// Maintain two reprojection buffers so we can read from the history buffer & write to the next one in parallel without unbinding
	TextureDX11 * m_tx_final;				// Final colour buffer output 

	RenderTargetDX11 *	m_rt[2];			// Maintain two render targets for efficiency: { { reproj[0], final }, { reproj[1], final } }

	ShaderDX11::ShaderParameterIndex	m_param_ps_temporal_deferred;
};