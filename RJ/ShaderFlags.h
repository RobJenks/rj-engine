#pragma once


// Type used to store shader flag data
typedef unsigned int ShaderFlags;


// Enumeration of bit-flags used to classify shader groups in the engine render queue
enum class ShaderFlag : ShaderFlags
{
	ShaderTypeGeometry			= (1 << 0),
	ShaderTypeUI				= (1 << 1)
};

