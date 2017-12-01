#pragma once

#ifndef __SkinnedNormalMapShaderH__
#define __SkinnedNormalMapShaderH__

#include "DX11_Core.h"
#include "Rendering.h"
#include "Light.h"
#include "SkinnedModel.h"


// This class has no special alignment requirements
class SkinnedNormalMapShader
{
private:

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

	Result Initialise(Rendering::RenderDeviceType *, HWND);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename);
	Result							InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename);
	Result							InitialiseCommonConstantBuffers(Rendering::RenderDeviceType  *device);

	void Shutdown();
	
	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV Render(Rendering::RenderDeviceContextType  *deviceContext, SkinnedModelInstance &model,
					XMFLOAT3 eyepos, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);


private:

	Result SetPerFrameShaderParameters(Rendering::RenderDeviceContextType  *deviceContext);

	Result SetPerObjectShaderParameters(	Rendering::RenderDeviceContextType  *deviceContext, SkinnedModelInstance &model, 
											XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);

private:
	
	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11Buffer			* m_perFrameBuffer;
	ID3D11Buffer			* m_perObjectBuffer;
	ID3D11Buffer			* m_perSubsetBuffer;
};


#endif