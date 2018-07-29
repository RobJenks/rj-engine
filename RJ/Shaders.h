#pragma once

#include <string>


class Shaders
{
public:

	static const std::string StandardVertexShader;
	static const std::string StandardPixelShader;

	static const std::string FullScreenQuadVertexShader;

	static const std::string DeferredGeometryPixelShader;
	static const std::string DeferredLightingPixelShader;
	static const std::string DeferredLightingDebug;

	static const std::string BasicTextureVertexShader;
	static const std::string BasicTexturePixelShader;

	static const std::string SDFDecalDirectVertexShader;
	static const std::string SDFDecalDeferredVertexShader;
	static const std::string SDFDecalDirectPixelShader;
	static const std::string SDFDecalDeferredPixelShader;

	static const std::string MotionBlurTileGen;
	static const std::string MotionBlurNeighbourhood;
	static const std::string MotionBlurGather;

	static const std::string TemporalReprojection;

	static const std::string ShadowMappingVertexShader;

};
