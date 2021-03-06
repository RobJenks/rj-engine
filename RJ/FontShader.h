#pragma once

#ifndef __FontShaderH__
#define __FontShaderH__

#include "DX11_Core.h"
#include <fstream>
#include "iShader.h"

// This class has no special alignment requirements
class FontShader 
{
private:
	struct VSBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct PSBufferType
	{
		XMFLOAT4 pixelColor;
	};

public:
	FontShader(void);
	~FontShader(void);

	Result Initialise(Rendering::RenderDeviceType * device, HWND hwnd);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename);
	Result							InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename);

	void Shutdown();

	Result Render(Rendering::RenderDeviceContextType * deviceContext, int indexCount, const FXMMATRIX worldMatrix,
						  const CXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, 
						  XMFLOAT4 pixelColor);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_inputlayout;
	ID3D11Buffer* m_cbuffer_vs;
	ID3D11Buffer* m_cbuffer_ps;
	ID3D11SamplerState* m_sampleState;

};


#endif