////////////////////////////////////////////////////////////////////////////////
// Filename: LightHighlightFadeShader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef __LightHighlightFadeShaderH__
#define __LightHighlightFadeShaderH__


//////////////
// INCLUDES //
//////////////
#include "DX11_Core.h"

#include <fstream>
#include "iShader.h"
class DXLocaliser;
using namespace std;


// This class has no special alignment requirements
class LightHighlightFadeShader : iShader
{
private:
	struct MatrixBufferType
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct LightHighlightFadeBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;
	};

public:
	LightHighlightFadeShader(const DXLocaliser *locale);
	LightHighlightFadeShader(const LightHighlightFadeShader&);
	~LightHighlightFadeShader();

	Result Initialise(ID3D11Device*, HWND);
	void Shutdown();

	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV Render(ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
		const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Sets the parameters specific to the light shader, i.e. light type / direction / colour
	Result SetLightParameters(XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor);

private:
	Result InitialiseShader_SM_All(ID3D11Device*, HWND, const char*, const char*);	// Initialise the shader; applicable to all shader model versions

	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const char*);

	Result XM_CALLCONV SetShaderParameters(ID3D11DeviceContext *deviceContext, const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix,
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

	XMFLOAT3				m_lightdirection;
	XMFLOAT4				m_ambientcolour;
	XMFLOAT4				m_diffusecolour;
};

#endif