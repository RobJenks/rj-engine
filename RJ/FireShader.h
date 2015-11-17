#pragma once

#ifndef __FireShaderH__
#define __FireShaderH__


#include "DX11_Core.h"


#include <fstream>
#include "iShader.h"
using namespace std;

// This class has no special alignment requirements
class FireShader 
{
private:
	struct MatrixBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct NoiseBufferType
	{
		float frameTime;
		XMFLOAT3 scrollSpeeds;
		XMFLOAT3 scales;
		float padding;
	};

	struct DistortionBufferType
	{
		XMFLOAT2 distortion1;
		XMFLOAT2 distortion2;
		XMFLOAT2 distortion3;
		float distortionScale;
		float distortionBias;
	};

public:
	FireShader(const DXLocaliser *locale);
	FireShader(const FireShader &other);
	~FireShader(void);

	Result Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	Result Render(ID3D11DeviceContext* deviceContext, int indexCount, const FXMMATRIX worldMatrix, const CXMMATRIX viewMatrix, 
							 const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
							 ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, float frameTime,
							 XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, XMFLOAT2 distortion2,
							 XMFLOAT2 distortion3, float distortionScale, float distortionBias);

private:
	Result InitialiseShader(ID3D11Device*, HWND, const char*, const char*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename);

	Result SetShaderParameters(ID3D11DeviceContext* deviceContext, const FXMMATRIX worldMatrix, const CXMMATRIX viewMatrix,
										  const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
										  ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, 
										  float frameTime, XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, 
										  XMFLOAT2 distortion2, XMFLOAT2 distortion3, float distortionScale, 
										  float distortionBias);

	void RenderShader(ID3D11DeviceContext*, int);

private:
	const DXLocaliser *m_locale;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_noiseBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11SamplerState* m_sampleState2;
	ID3D11Buffer* m_distortionBuffer;

};



#endif