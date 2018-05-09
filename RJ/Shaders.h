#pragma once

#include <string>


class Shaders
{
public:

	static const std::string StandardVertexShader;
	static const std::string StandardPixelShader;

	static const std::string DeferredGeometryPixelShader;
	static const std::string DeferredLightingPixelShader;
	static const std::string DeferredLightingDebug;

	static const std::string BasicTextureVertexShader;
	static const std::string BasicTexturePixelShader;

};
