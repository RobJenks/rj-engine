////////////////////////////////////////////////////////////////////////////////
// Filename: LightShader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef __LightShaderH__
#define __LightShaderH__


//////////////
// INCLUDES //
//////////////
#include "DX11_Core.h"

#include <d3dx11async.h>
#include <fstream>
#include "iShader.h"
class DXLocaliser;
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: LightShader
////////////////////////////////////////////////////////////////////////////////
class LightShader : iShader
{
private:
	struct MatrixBufferType
	{
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct LightBufferType
	{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
		float padding;  // Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
	};

public:
	LightShader(const DXLocaliser *locale);
	LightShader(const LightShader&);
	~LightShader();

	Result Initialise(ID3D11Device*, HWND);
	void Shutdown();
	
	// Renders the shader.  Conforms to the iShader interface spec
	Result Render(	ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
					D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Sets the parameters specific to the light shader, i.e. light type / direction / colour
	Result SetLightParameters(D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor);


private:
	Result InitialiseShader_SM5(ID3D11Device*, HWND, const char*, const char*);	// Initialise a shader model 5 shader
	Result InitialiseShader_SM2(ID3D11Device*, HWND, const char*, const char*);	// Initialise a shader model 2 shader

	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const char*);

	Result SetShaderParameters(	ID3D11DeviceContext *deviceContext, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, 
								ID3D11ShaderResourceView* texture);

	void RenderShader(ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount);

private:
	const DXLocaliser		* m_locale;
	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_layout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_matrixBuffer;
	ID3D11Buffer			* m_lightBuffer;

	D3DXVECTOR3				m_lightdirection;
	D3DXVECTOR4				m_ambientcolour;
	D3DXVECTOR4				m_diffusecolour;
};

#endif