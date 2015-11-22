#pragma once

#ifndef __CubeMapTextureH__
#define __CubeMapTextureH__

#include "Texture.h"

// This class has no special alignment requirements
class CubeMapTexture : public Texture
{
public:
	CubeMapTexture(void);
	~CubeMapTexture(void);

	Result Initialise(ID3D11Device *device, const char *filename);
};


#endif