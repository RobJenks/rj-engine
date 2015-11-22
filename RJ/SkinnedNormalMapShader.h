#pragma once

#ifndef __SkinnedNormalMapShaderH__
#define __SkinnedNormalMapShaderH__

#include <windows.h>
#include "DX11_Core.h"

#include <fstream>
#include "iShader.h"
#include "Light.h"
#include "SkinnedModel.h"
class DXLocaliser;
using namespace std;


// This class has no special alignment requirements
class SkinnedNormalMapShader //: iShader
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
	SkinnedNormalMapShader(const DXLocaliser *locale);
	SkinnedNormalMapShader(const SkinnedNormalMapShader&);
	~SkinnedNormalMapShader();

	Result Initialise(ID3D11Device*, HWND);
	void Shutdown();
	
	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV Render(ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model,
					XMFLOAT3 eyepos, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);


private:
	Result InitialiseShader_SM5(ID3D11Device*, HWND, const char*, const char*);	// Initialise a shader model 5 shader
	Result InitialiseShader_SM2(ID3D11Device*, HWND, const char*, const char*);	// Initialise a shader model 2 shader

	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const char*);

	Result SetPerFrameShaderParameters(		ID3D11DeviceContext *deviceContext, DirectionalLight *lights3, XMFLOAT3 eyepos);

	Result SetPerObjectShaderParameters(	ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model, 
											XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);

	void RenderShader(ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model);

private:
	const DXLocaliser		* m_locale;
	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_layout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_perFrameBuffer;
	ID3D11Buffer			* m_perObjectBuffer;
	ID3D11Buffer			* m_perSubsetBuffer;

	DirectionalLight		* m_lights;					// Hardcoded to be an array of 3 directional lights
};


#endif