#pragma once

#ifndef __CubeMapTextureH__
#define __CubeMapTextureH__

#include "texture.h"

class CubeMapTexture :
	public Texture
{
public:
	CubeMapTexture(void);
	~CubeMapTexture(void);

	Result Initialise(ID3D11Device *device, const char *filename);
};


#endif