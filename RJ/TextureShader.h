#pragma once

#ifndef __TextureShaderH__
#define __TextureShaderH__

#include "DX11_Core.h"


#include <fstream>
#include "iShader.h"
using namespace std;


// This class has no special alignment requirements
class TextureShader 
{
private:
	struct MatrixBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

public:
	TextureShader(const DXLocaliser *locale);
	TextureShader(const TextureShader &other);
	~TextureShader(void);

	Result Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	Result Render(ID3D11DeviceContext* deviceContext, int indexCount, const FXMMATRIX worldMatrix, 
						  const CXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

private:
	Result InitialiseShader(ID3D11Device*, HWND, const char*, const char*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename);

	Result SetShaderParameters(ID3D11DeviceContext*, const FXMMATRIX, const CXMMATRIX, const CXMMATRIX, ID3D11ShaderResourceView*);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	const DXLocaliser *m_locale;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;

};


#endif