#pragma once

#include "CompilerSettings.h"
#include "IntVector.h"
#include "ShaderDX11.h"
#include "Viewport.h"
#include "PostProcessComponent.h"
class DeferredRenderProcess;
class DeferredGBuffer;
class RenderTargetDX11;
class TextureDX11;
class ConstantBufferDX11;
class PipelineStateDX11;


class PostProcessMotionBlur : public PostProcessComponent
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
	
	// Execute the post-process over the source buffer.  Returns a pointer to the final buffer
	// following post-processing
	TextureDX11 *								Execute(TextureDX11 *source_colour, TextureDX11 *source_vel);

	// Return the final post-processed result
	CMPINLINE TextureDX11 *						GetRenderedOutput(void) const { return m_tx_gather; }

	// Return intermediate phase outputs
	CMPINLINE TextureDX11 *						GetTileGenerationPhaseResult(void) const { return m_tx_tilegen; }
	CMPINLINE TextureDX11 *						GetNeighbourhoodDeterminationResult(void) const { return m_tx_neighbour; }

	// Respond to a change in shader configuration or a reload of shader bytecode
	void										ShadersReloaded(void);

	// Destructor
	~PostProcessMotionBlur(void);

private:

	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);
	void InitialiseRenderGeometry(void);

	void InitialiseTileGenerationPipeline(void);
	void InitialiseNeighbourhoodCalculationPipeline(void);
	void InitialiseGatherPhasePipeline(void);

	void ExecuteTileGenerationPass(TextureDX11 *source_vel);
	void ExecuteNeighbourhoodDeterminationPass(TextureDX11 *velocity_tile_data);
	void ExecuteGatherPass(	TextureDX11 *source_colour, TextureDX11 *source_depth, TextureDX11 * source_vel, 
							TextureDX11 *velocity_neighbourhood_buffer);

private:

	static const unsigned int				DEFAULT_TILE_SCALING_FACTOR = 2U;
	static const std::string				RT_NAME_TILEGEN;
	static const std::string				RT_NAME_NEIGHBOUR;
	static const std::string				RT_NAME_GATHER;
	static const std::string				TX_NAME_TILEGEN;
	static const std::string				TX_NAME_NEIGHBOUR;
	static const std::string				TX_NAME_GATHER;
	static const std::string				RP_NAME_TILEGEN;
	static const std::string				RP_NAME_NEIGHBOUR;
	static const std::string				RP_NAME_GATHER;
	
	DeferredRenderProcess *					m_renderprocess;

	ShaderDX11 *							m_vs_quad;
	ShaderDX11 *							m_ps_tilegen;
	ShaderDX11 *							m_ps_neighbourhood;
	ShaderDX11 *							m_ps_gather;

	RenderTargetDX11 *						m_rt_tilegen;
	RenderTargetDX11 *						m_rt_neighbour;
	RenderTargetDX11 *						m_rt_gather;

	TextureDX11 *							m_tx_tilegen;
	TextureDX11 *							m_tx_neighbour;
	TextureDX11 *							m_tx_gather;

	PipelineStateDX11 *						m_pipeline_tilegen;
	PipelineStateDX11 *						m_pipeline_neighbour;
	PipelineStateDX11 *						m_pipeline_gather;

	Viewport								m_downsampled_viewport;
	XMMATRIX								m_downsampled_fullscreen_transform;

	ShaderDX11::ShaderParameterIndex		m_param_ps_tilegen_deferred;
	ShaderDX11::ShaderParameterIndex		m_param_ps_tilgen_velocitybuffer;
	ShaderDX11::ShaderParameterIndex		m_param_ps_neighbour_deferred;
	ShaderDX11::ShaderParameterIndex		m_param_ps_neighbour_tilebuffer;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_deferred;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_colour;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_depth;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_velocity;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_vel_neighbourhood;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_noise_tex;
	ShaderDX11::ShaderParameterIndex		m_param_ps_gather_noise_data;


	unsigned int							m_tilesize_k;
	IntegralVector2<unsigned int>			m_tiled_dimensions;

};