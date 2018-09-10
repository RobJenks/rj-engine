#pragma once

#include "../Definitions/CppHLSLLocalisation.hlsl.h"

class InstanceFlags
{
public:

	// 32-bit flag type.  Note this must match the RM_Instance definition
	typedef _uint32				Type;

	// Instance flags
	static const Type			INSTANCE_FLAG_SHADOW_CASTER			= (1 << 0);




	// Default instance flags
	static const Type			DEFAULT_INSTANCE_FLAGS				= 0U;


};