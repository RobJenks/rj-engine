#pragma once

#ifndef __ParticleShaderH__
#define __ParticleShaderH__

#include "DX11_Core.h"
#include "Rendering.h"

// This class has no special alignment requirements
class ParticleShader
{
private:
	struct VSBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

public:
	ParticleShader(void);
	~ParticleShader(void);

	Result Initialise(Rendering::RenderDeviceType * device, HWND hwnd);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename);
	Result							InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename);

	void Shutdown();

	Result RJ_XM_CALLCONV Render(Rendering::RenderDeviceContextType * deviceContext, int indexCount, const FXMMATRIX worldMatrix,
		const CXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_inputlayout;
	ID3D11Buffer* m_cbuffer_vs;
	ID3D11SamplerState* m_sampleState;

};


#endif