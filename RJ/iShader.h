#pragma once

#ifndef __iShaderH__
#define __iShaderH__

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
#include "DX11_Core.h"
#include "GameDataExtern.h"

class iShader
{
public:

	// Renders the shader.
	virtual Result Render( ID3D11DeviceContext *deviceContext,	int vertexCount, int indexCount, int instanceCount, 
							D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)		= 0;


	// Static method; returns a full filename for the specified shader file
	CMPINLINE static std::string ShaderFilename(const std::string & file)
	{
		return concat(D::DATA)("/Shaders/")(file).str();
	}

	iShader(void);
	~iShader(void);
};


#endif