#include "Shaders.h"


const std::string Shaders::StandardVertexShader			= "VS_Standard";
const std::string Shaders::StandardPixelShader			= "PS_Standard";

const std::string Shaders::DeferredGeometryPixelShader	= "PS_Deferred_Geometry";
const std::string Shaders::DeferredLightingPixelShader	= "PS_Deferred_Lighting";
const std::string Shaders::DeferredLightingDebug		= "PS_Deferred_Debug";

const std::string Shaders::BasicTextureVertexShader		= "VS_Basic_Texture";
const std::string Shaders::BasicTexturePixelShader		= "PS_Basic_Texture";

const std::string Shaders::SDFDecalVertexShader			= "VS_SDFDecal";
const std::string Shaders::SDFDecalPixelShader			= "PS_SDFDecal"; // = "PS_MSDFDecal"; [for multi-channel SDF, not fully polished]

