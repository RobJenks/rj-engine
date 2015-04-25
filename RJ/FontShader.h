#pragma once

#ifndef __FontShaderH__
#define __FontShaderH__

#include "DX11_Core.h"

#include <d3dx11async.h>
#include <fstream>
#include "iShader.h"
class DXLocaliser;
using namespace std;


class FontShader 
{
private:
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct PixelBufferType
	{
		D3DXVECTOR4 pixelColor;
	};

public:
	FontShader(const DXLocaliser *locale);
	FontShader(const FontShader &other);
	~FontShader(void);

	Result Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	Result Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, 
						  D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, 
						  D3DXVECTOR4 pixelColor);

private:
	Result InitialiseShader(ID3D11Device*, HWND, const char*, const char*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename);

	Result SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, D3DXVECTOR4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	const DXLocaliser *m_locale;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_pixelBuffer;

};


#endif