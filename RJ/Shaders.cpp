#include "Shaders.h"


const std::string Shaders::StandardVertexShader			= "VS_Standard";
const std::string Shaders::StandardPixelShader			= "PS_Standard";

const std::string Shaders::FullScreenQuadVertexShader	= "VS_Quad";

const std::string Shaders::DeferredGeometryPixelShader	= "PS_Deferred_Geometry";
const std::string Shaders::DeferredLightingPixelShader	= "PS_Deferred_Lighting";
const std::string Shaders::DeferredLightingDebug		= "PS_Deferred_Debug";

const std::string Shaders::BasicTextureVertexShader		= "VS_Basic_Texture";
const std::string Shaders::BasicTexturePixelShader		= "PS_Basic_Texture";

const std::string Shaders::SDFDecalDirectVertexShader	= "VS_SDFDecal_Direct";
const std::string Shaders::SDFDecalDeferredVertexShader	= "VS_SDFDecal_Deferred";
const std::string Shaders::SDFDecalDirectPixelShader	= "PS_SDFDecal_Direct"; // = "PS_MSDFDecal"; [for multi-channel SDF, not fully polished]
const std::string Shaders::SDFDecalDeferredPixelShader	= "PS_SDFDecal_Deferred"; 

const std::string Shaders::MotionBlurTileGen			= "PS_MotionBlur_Tilegen";
const std::string Shaders::MotionBlurNeighbourhood		= "PS_MotionBlur_Neighbourhood";
const std::string Shaders::MotionBlurGather				= "PS_MotionBlur_Gather";

const std::string Shaders::TemporalReprojection			= "PS_Temporal";

const std::string Shaders::ShadowMappingVertexShader	= "VS_ShadowMap";

