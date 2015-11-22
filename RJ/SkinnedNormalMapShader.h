#pragma once

#ifndef __SkinnedNormalMapShaderH__
#define __SkinnedNormalMapShaderH__

#include "DX11_Core.h"
#include "Light.h"
#include "SkinnedModel.h"


// This class has no special alignment requirements
class SkinnedNormalMapShader
{
private:
	struct PerFrameBuffer
	{
		DirectionalLight			gDirLights[3];
		XMFLOAT3					gEyePosW;

		float						gFogStart;
		float						gFogRange;
		XMFLOAT4					gFogColor; 
		XMFLOAT3					padding;
	};

	struct PerObjectBuffer
	{
		XMMATRIX					gWorld;
		XMMATRIX					gWorldInvTranspose;
		XMMATRIX					gWorldViewProj;
		XMMATRIX					gWorldViewProjTex;
		XMMATRIX					gTexTransform;
		XMMATRIX					gShadowTransform; 

		XMFLOAT4X4					gBoneTransforms[96];
	};

	struct PerSubsetBuffer
	{
		SkinnedModel::SM_Material	gMaterial;
	};


public:
	SkinnedNormalMapShader(void);
	~SkinnedNormalMapShader();

	Result Initialise(ID3D11Device*, HWND);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(ID3D11Device *device, std::string filename);
	Result							InitialisePixelShader(ID3D11Device *device, std::string filename);
	Result							InitialiseCommonConstantBuffers(ID3D11Device *device);

	void Shutdown();
	
	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV Render(ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model,
					XMFLOAT3 eyepos, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);


private:

	Result SetPerFrameShaderParameters(		ID3D11DeviceContext *deviceContext, DirectionalLight *lights3, XMFLOAT3 eyepos);

	Result SetPerObjectShaderParameters(	ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model, 
											XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);

private:
	
	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11Buffer			* m_perFrameBuffer;
	ID3D11Buffer			* m_perObjectBuffer;
	ID3D11Buffer			* m_perSubsetBuffer;

	DirectionalLight		* m_lights;					// Hardcoded to be an array of 3 directional lights
};


#endif