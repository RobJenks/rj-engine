#pragma once

class ShaderParameter
{
public:

	enum class Type
	{
		ConstantBuffer = 0,			// Standard constant buffer
		StructuredBuffer,			// Structured buffer
		Texture,					// Standard texture resource
		Sampler,					// Texture sampler
		RWTexture,					// GPU-read-write texture, i.e. allowing write operations within the shader
		RWBuffer					// GPU-read-write structure buffer, i.e. allowing write operations within the shader
	};


private:



};