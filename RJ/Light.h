#pragma once

#ifndef __LightH__
#define __LightH__

#include "ALIGN16.h"
#include "DX11_Core.h"
#include "Data\\Shaders\\light_definition.h"

class Light : public ALIGN16<Light>
{
public:

	// Static counter used to assign unique light IDs
	static unsigned int				LAST_ID;

	// Lighting data
	LightData						Data;

	// Default constructor
	Light(void);

	// Custom copy constructor
	Light(const Light & source);

	// Method to return a new unique ID
	static unsigned int				NewUniqueID(void);

	// Initialise a light to the specified type
	void							InitialiseDirectionalLight(const XMFLOAT3 & direction, const XMFLOAT3 & colour, float ambient, float diffuse, float specular);
	void							InitialisePointLight(	const XMFLOAT3 & position, const XMFLOAT3 & direction, const XMFLOAT3 & colour, float range, float ambient, 
															float diffuse, float specular, const AttenuationData & attenuation);

	// Default destructor
	~Light(void);

protected:

};




#endif