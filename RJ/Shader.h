#pragma once

#include <string>
#include <limits>

class Shader
{
public:

	enum class Type
	{
		VertexShader = 0,
		PixelShader,
		HullShader,
		DomainShader,
		GeometryShader,
		ComputeShader, 

		SHADER_TYPE_COUNT
	};

	static std::string ShaderTypeToString(Shader::Type type);

	typedef unsigned int SlotID;
	static const SlotID NO_SLOT_ID = (std::numeric_limits<SlotID>::max)();

private:


};