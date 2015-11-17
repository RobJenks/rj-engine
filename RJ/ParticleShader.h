#pragma once

#ifndef __ParticleShaderH__
#define __ParticleShaderH__


#include "DX11_Core.h"


#include <fstream>
#include "ErrorCodes.h"
#include "iShader.h"
class DXLocaliser;
using namespace std;

// This class has no special alignment requirements
class ParticleShader 
{
	private:
		struct MatrixBufferType
		{
			XMFLOAT4X4 world;
			XMFLOAT4X4 view;
			XMFLOAT4X4 projection;
		};

	public:
		ParticleShader(const DXLocaliser *locale);
		ParticleShader(const ParticleShader&);
		~ParticleShader();

		Result Initialize(ID3D11Device*, HWND);
		void Shutdown();
		Result Render(ID3D11DeviceContext*, int, const FXMMATRIX, const CXMMATRIX, const CXMMATRIX, ID3D11ShaderResourceView*);

	private:
		Result InitializeShader_SM2(ID3D11Device*, HWND, const char *, const char *);
		Result InitializeShader_SM5(ID3D11Device*, HWND, const char *, const char *);
		void ShutdownShader();
		void OutputShaderErrorMessage(ID3D10Blob*, HWND, const char*);

		Result SetShaderParameters(ID3D11DeviceContext*, const FXMMATRIX, const CXMMATRIX, const CXMMATRIX, ID3D11ShaderResourceView*);
		void RenderShader(ID3D11DeviceContext*, int);

	private:
		const DXLocaliser		* m_locale;
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_pixelShader;
		ID3D11InputLayout* m_layout;
		ID3D11Buffer* m_matrixBuffer;
		ID3D11SamplerState* m_sampleState;
};



#endif