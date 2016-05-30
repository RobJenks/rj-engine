#pragma once

#ifndef __MaterialH__
#define __MaterialH__

#include "Data\\Shaders\\material_definition.h"

class Material
{
public:

	// Maximum number of materials that can be rendered in one pass
	static const unsigned int	MATERIAL_LIMIT;

	// Material data; held in structure common to both cpp and hlsl builds
	MaterialData				Data;


	// Default constructor
	Material(void);

	// Default destructor
	~Material(void);
};



#endif


