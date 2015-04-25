#pragma once

#ifndef __iShaderH__
#define __iShaderH__

#include "DX11_Core.h"

#include "ErrorCodes.h"

class iShader
{
public:

	// Renders the shader.
	virtual Result Render( ID3D11DeviceContext *deviceContext,	int vertexCount, int indexCount, int instanceCount, 
							D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)		= 0;

	iShader(void);
	~iShader(void);
};



#endif