#pragma once

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

	typedef unsigned int SlotID;


private:


};