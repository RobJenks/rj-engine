#pragma once

#include "CompilerSettings.h"
#include "IntVector.h"
#include "ShaderDX11.h"
class DeferredRenderProcess;
class RenderTargetDX11;
class TextureDX11;
class ConstantBufferDX11;
class PipelineStateDX11;


class PostProcessMotionBlur
{
public:

	// Initialise the postprocess and all required resources
	PostProcessMotionBlur(void);
	PostProcessMotionBlur(DeferredRenderProcess * render_process);

	// Reinitialise based on a change to the effect configuration
	void PerformPostConfigInitialisation(void);

	// Tiled dimensions (per-dimension tile count, K) for this postprocess
	CMPINLINE unsigned int						GetTileScalingFactor(void) { return m_tilesize_k; }
	CMPINLINE IntegralVector2<unsigned int>		GetTiledDimensions(void) { return m_tiled_dimensions; }
	void										SetTileScalingFactor(unsigned int K);
	


	// Destructor
	~PostProcessMotionBlur(void);

private:

	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);


private:

	static const unsigned int				DEFAULT_TILE_SCALING_FACTOR = 2U;
	static const std::string				RT_NAME_TILEGEN;
	static const std::string				RT_NAME_NEIGHBOUR;
	static const std::string				RT_NAME_GATHER;
	static const std::string				TX_NAME_TILEGEN;
	static const std::string				TX_NAME_NEIGHBOUR;
	static const std::string				TX_NAME_GATHER;

	DeferredRenderProcess *					m_renderprocess;

	ShaderDX11 *							m_vs;
	ShaderDX11 *							m_ps_tilegen;
	ShaderDX11 *							m_ps_neighbourhood;
	ShaderDX11 *							m_ps_gather;

	RenderTargetDX11 *						m_rt_tilegen;
	RenderTargetDX11 *						m_rt_neighbour;
	RenderTargetDX11 *						m_rt_gather;

	TextureDX11 *							m_tx_tilegen;
	TextureDX11 *							m_tx_neighbour;
	TextureDX11 *							m_tx_gather;

	ShaderDX11::ShaderParameterIndex		m_param_vs_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_tilegen_deferred;
	ShaderDX11::ShaderParameterIndex		m_param_ps_neighbour_deferred;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_deferred;

	unsigned int							m_tilesize_k;
	IntegralVector2<unsigned int>			m_tiled_dimensions;

};