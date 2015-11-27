#pragma once

#ifndef __iShaderH__
#define __iShaderH__

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
#include "DX11_Core.h"
#include "GameDataExtern.h"


// This class has no special alignment requirements
class iShader
{
public:

	// Renders the shader.
	virtual Result XM_CALLCONV Render(	ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
										const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)		= 0;


	// Static method; returns a full filename for the specified shader file
	CMPINLINE static std::string ShaderFilename(const std::string & file)
	{
		return concat(Game::ExePath)("\\Data\\Shaders\\")(file).str();
	}

	iShader(void);
	~iShader(void);
};


#endif