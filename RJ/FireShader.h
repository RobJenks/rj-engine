#pragma once

#ifndef __FireShaderH__
#define __FireShaderH__

#include "DX11_Core.h"
#include "iShader.h"
#include "Rendering.h"

// This class has no special alignment requirements
class FireShader 
{
private:
	struct VSBufferType1
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct VSBufferType2
	{
		float frameTime;
		XMFLOAT3 scrollSpeeds;
		XMFLOAT3 scales;
		float padding;
	};

	struct PSBufferType
	{
		XMFLOAT2 distortion1;
		XMFLOAT2 distortion2;
		XMFLOAT2 distortion3;
		float distortionScale;
		float distortionBias;
	};

public:
	FireShader(void);
	~FireShader(void);

	Result Initialise(Rendering::RenderDeviceType * device, HWND hwnd);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename);
	Result							InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename);

	void Shutdown();

	Result XM_CALLCONV Render(Rendering::RenderDeviceContextType * deviceContext, int indexCount, const FXMMATRIX worldMatrix, const CXMMATRIX viewMatrix,
							 const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
							 ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, float frameTime,
							 XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, XMFLOAT2 distortion2,
							 XMFLOAT2 distortion3, float distortionScale, float distortionBias);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_inputlayout;
	ID3D11Buffer* m_cbuffer_vs1;
	ID3D11Buffer* m_cbuffer_vs2;
	ID3D11Buffer* m_cbuffer_ps;
	ID3D11SamplerState* m_sampleState1;
	ID3D11SamplerState* m_sampleState2;

};



#endif